import argparse
import math
import os
import re
import warnings

import btllib
import pandas as pd
import torch
import torchinfo
from aiedit_torch_extensions import get_model_input


def positional_encoding(max_length: int, dim: int):
    position = torch.arange(max_length).unsqueeze(1)
    div_term = torch.arange(0, dim, 2)
    div_term = torch.exp(div_term * (-math.log(10000.0) / dim))
    pos_enc = torch.zeros(max_length, dim)
    pos_enc[:, 0::2] = torch.sin(position * div_term)
    if dim % 2 != 0:
        pos_enc[:, 1::2] = torch.cos(position * div_term)[:, 0:-1]
    else:
        pos_enc[:, 1::2] = torch.cos(position * div_term)
    return pos_enc


class Model(torch.nn.Module):

    def __init__(
        self,
        num_seeds: int,
        max_indels: int,
        max_length: int = 1000,
    ):
        super(Model, self).__init__()
        model_dim = num_seeds + 2 * max_indels + 1
        self.__probs_enc = positional_encoding(max_length, model_dim)
        self.__edits_enc = positional_encoding(max_length, 5)
        self.register_buffer("probs_enc", self.__probs_enc)
        self.register_buffer("edits_enc", self.__edits_enc)
        self.__probs_attn = torch.nn.MultiheadAttention(model_dim, 1)
        self.__seeds_attn = torch.nn.MultiheadAttention(
            embed_dim=model_dim,
            num_heads=1,
            kdim=num_seeds,
            vdim=num_seeds,
        )
        self.__edits_attn = torch.nn.MultiheadAttention(
            embed_dim=5,
            num_heads=1,
            kdim=model_dim,
            vdim=model_dim,
        )
        self.__masked_attn = torch.nn.MultiheadAttention(5, 1)
        self.__probs_norm = torch.nn.LayerNorm(model_dim)
        self.__seeds_norm = torch.nn.LayerNorm(model_dim)
        self.__edits_norm = torch.nn.LayerNorm(5)
        self.__out = torch.nn.Linear(5, 5)
        self.__input_sizes = [(100, model_dim), (25, num_seeds), (max_indels, 5)]

    def summary(self):
        torchinfo.summary(self, input_size=self.__input_sizes)

    def forward(self, x_probs, x_seeds, x_edits):
        x_s = x_probs + self.__probs_enc[: x_probs.size(0), :]
        x_s = self.__probs_norm(x_s + self.__probs_attn(x_s, x_s, x_s)[0])
        x_s = self.__seeds_norm(x_s + self.__seeds_attn(x_s, x_seeds, x_seeds)[0])
        mask = torch.ones(x_edits.size(0), x_edits.size(0))
        mask = torch.triu(mask * float("-inf"), diagonal=1)
        x_t = x_edits + self.__edits_enc[: x_edits.size(0), :]
        x_t, _ = self.__masked_attn(x_t, x_t, x_t, attn_mask=mask, is_causal=True)
        y = self.__edits_norm(x_t + self.__edits_attn(x_t, x_s, x_s)[0])
        return self.__out(y)


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("-r", help="path to sequences", required=True)
    parser.add_argument("-e", help="path to variants list file", required=True)
    parser.add_argument("-b", help="path to counting bloom filter", required=True)
    parser.add_argument("-m", help="path to histogram model file", required=True)
    parser.add_argument("-s", help="path to seeds file", required=True)
    parser.add_argument("-o", help="path to model checkpoint file", required=True)
    parser.add_argument("-i", help="maximum indel length", type=int, default=10)
    parser.add_argument("-n", help="number of epochs per read", type=int, default=5)
    return parser.parse_args()


def read_seeds(path: str) -> list[str]:
    with open(path) as file:
        seeds = [line.strip() for line in file]
    assert len(set(map(len, seeds))) == 1, "seeds should be the same length"
    x_seeds = torch.empty(len(seeds[0]), len(seeds))
    for i in range(x_seeds.size(0)):
        for j in range(x_seeds.size(1)):
            x_seeds[i, j] = int(seeds[j][i])
    return seeds, x_seeds


def read_vars(path: str) -> tuple[int, dict[str, pd.DataFrame]]:
    vars = pd.read_csv(path, delimiter=r"\s+", nrows=2000)
    num_vars = len(vars)
    vars = {str(seq_name): seq_vars for seq_name, seq_vars in vars.groupby("Seq_name")}
    return num_vars, vars


def read_histogram(path: str) -> pd.DataFrame:
    hist = pd.read_csv(path, delimiter=r"\s+", index_col=0)
    hist.loc[0] = [0, 1, 0, 0]
    norm_cols = ["error", "heterozygous", "homozygous"]
    row_sum = hist[norm_cols].sum(axis=1)
    hist[norm_cols] = hist[norm_cols].div(row_sum, axis=0)
    return hist


def get_targets(vars: pd.DataFrame, pos_diff: int):
    end_pos = vars.iloc[-1]["Seq_pos"] + vars.iloc[-1]["error_length"]
    num_edits = end_pos - vars.iloc[0]["Seq_pos"]
    targets = torch.zeros(num_edits + 2, 5)
    targets[-1, 4] = 1.0
    for _, row in vars.iterrows():
        seq_pos = row["Seq_pos"] - vars.iloc[0]["Seq_pos"]
        if row["error_type"] == "mis":
            targets[seq_pos : seq_pos + row["error_length"], 0] = 0.0
            targets[seq_pos : seq_pos + row["error_length"], 1] = 1.0
        elif row["error_type"] == "ins":
            targets[seq_pos : seq_pos + row["error_length"], 0] = 0.0
            targets[seq_pos : seq_pos + row["error_length"], 2] = 1.0
            pos_diff += row["error_length"]
        elif row["error_type"] == "del":
            targets[seq_pos : seq_pos + row["error_length"], 0] = 0.0
            targets[seq_pos : seq_pos + row["error_length"], 3] = 1.0
            pos_diff -= row["error_length"]
    return targets, pos_diff


def validiate_sample(x, y, num_seeds: int) -> bool:
    max_indels = x.size(1) - 1 - num_seeds
    x_seeds = x[:, 1 : num_seeds + 1]
    x_ins = x[:, -2 * max_indels : -max_indels]
    x_del = x[:, -max_indels:]
    if y[:, 1].any() and not (x_seeds < 0.5).any():
        return False
    if y[:, 2].any() and not (x_ins < 0.5).any():
        return False
    if y[:, 3].any() and not (x_del < 0.5).any():
        return False
    return True


def generate_data(
    reads_path: str,
    vars: dict[str, pd.DataFrame],
    cbf,
    hist: pd.DataFrame,
    seeds: list[str],
    max_ind: int,
):
    probs = hist["error"].tolist()
    seq_reader = btllib.SeqReader(reads_path, btllib.SeqReaderFlag.LONG_MODE)  # type: ignore
    for record in seq_reader:
        match = re.search(r"F_(\d+)_\d+_\d+", record.id)
        if not match:
            warnings.warn(f"invalid read id: {record.id}")
            continue
        if record.id not in vars:
            warnings.warn(f"read has no errors: {record.id}")
            continue
        k = len(seeds[0])
        read_vars = vars[record.id].sort_values("Seq_pos")
        read_vars["Seq_pos"] += int(match.group(1))
        pos_ends = (read_vars["Seq_pos"] + read_vars["error_length"]).shift().fillna(0)
        read_vars["group"] = (read_vars["Seq_pos"] - pos_ends > 2 * k).cumsum()
        x_read, y_read = [], []
        pos_diff = 0
        for _, group in read_vars.groupby("group"):
            start = max(group.iloc[0]["Seq_pos"] - k + 1, 0) + pos_diff
            end = group.iloc[-1]["Seq_pos"] + group.iloc[-1]["error_length"] + pos_diff
            x = get_model_input(record.seq, start, end, seeds, max_ind, int(cbf), probs)
            y, pos_diff = get_targets(group, pos_diff)
            if validiate_sample(x, y, len(seeds)):
                x_read.append(x)
                y_read.append(y)
        yield record.id, x_read, y_read


def weighted_ce_loss(logits, targets, reduction_factor=0.01):
    ce_loss = torch.nn.functional.cross_entropy(logits, targets, reduction="none")
    ce_loss[(logits.argmax(dim=1) == targets) & (targets == 0)] *= reduction_factor
    return ce_loss.mean()


def train(model, optimizer, read_id, x, x_seeds, y, num_epochs):
    loss_history = []
    for i in range(num_epochs):
        epoch_loss = 0
        for x_batch, y_true in zip(x, y):
            optimizer.zero_grad()
            y_pred = model(x_batch, x_seeds, y_true)[:-1, :]
            loss = weighted_ce_loss(y_pred, y_true[1:, :].argmax(dim=1))
            loss.backward()
            optimizer.step()
            epoch_loss += loss.item()
        epoch_loss /= len(x)
        print(f"[{read_id}] Epoch {i + 1}/{num_epochs}: loss = {epoch_loss:.4f}")
        loss_history.append(epoch_loss)
    return loss_history


def main():
    args = parse_args()
    seeds, x_seeds = read_seeds(args.s)
    hist = read_histogram(args.m)
    model = Model(len(seeds), args.i)
    optimizer = torch.optim.AdamW(model.parameters())
    if os.path.isfile(args.o):
        checkpoint = torch.load(args.o, weights_only=True)
        model.load_state_dict(checkpoint["model_state_dict"])
        optimizer.load_state_dict(checkpoint["optimizer_state_dict"])
    print("Reading variants file...")
    num_vars, vars = read_vars(args.e)
    print(f"Number of variants = {num_vars}")
    print(f"Number of reads = {len(vars)}")
    print("Reading CBF...")
    cbf = btllib.CountingBloomFilter8(args.b)  # type: ignore
    print(f"CBF FPR = {cbf.get_fpr()}")
    print("Seeds:", *seeds, sep=os.linesep)
    data_generator = generate_data(args.r, vars, cbf, hist, seeds, args.i)
    for read_id, x, y in data_generator:
        train(model, optimizer, read_id, x, x_seeds, y, args.n)
        state_dict = {
            "model_state_dict": model.state_dict(),
            "optimizer_state_dict": optimizer.state_dict(),
        }
        torch.save(state_dict, args.o)


if __name__ == "__main__":
    main()
