import argparse
import os
import re
import warnings

import aiedit_torch_extensions
import btllib
import pandas as pd
import torch
import torchinfo


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
    return seeds


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


def load_model(
    num_seeds: int,
    max_ind: int,
    path: str,
) -> tuple[torch.nn.Module, torch.optim.AdamW]:
    model = torch.nn.Sequential(
        torch.nn.Conv1d(num_seeds + 2 * max_ind + 1, 64, 7, padding="same"),
        torch.nn.ReLU(),
        torch.nn.Conv1d(64, 128, 7, padding="same"),
        torch.nn.ReLU(),
        torch.nn.Conv1d(128, 64, 7, padding="same"),
        torch.nn.ReLU(),
        torch.nn.Conv1d(64, max_ind + 3, 7, padding="same"),
    )
    optimizer = torch.optim.AdamW(model.parameters())
    if os.path.isfile(path):
        checkpoint = torch.load(path, weights_only=True)
        model.load_state_dict(checkpoint["model_state_dict"])
        optimizer.load_state_dict(checkpoint["optimizer_state_dict"])
    return model, optimizer


def print_model_summary(model):
    num_channels = next(model.parameters()).size(1)
    torchinfo.summary(model, input_size=(1, num_channels, 100))


def get_targets(vars: pd.DataFrame, seq_len: int, k: int, max_indels):
    y = torch.zeros(max_indels + 3, seq_len)
    y[0, :] = 1.0
    pos_diff = 0
    for _, row in vars.iterrows():
        seq_pos = row["Seq_pos"] - vars.iloc[0]["Seq_pos"] + pos_diff + 1
        if row["error_type"] == "mis":
            y[0, seq_pos : seq_pos + row["error_length"]] = 0.0
            y[1, seq_pos : seq_pos + row["error_length"]] = 1.0
        elif row["error_type"] == "ins":
            y[0, seq_pos : seq_pos + row["error_length"]] = 0.0
            y[2, seq_pos : seq_pos + row["error_length"]] = 1.0
            pos_diff += row["error_length"]
        elif row["error_type"] == "del":
            num_ins = min(row["error_length"], max_indels)
            y[0, seq_pos - 1] = 0.0
            y[num_ins + 2, seq_pos - 1] = 1.0
            pos_diff -= row["error_length"]
    return y


def filter_samples(x, y, k):
    mask = torch.zeros(x.size(1)).bool()
    num_clean = torch.nn.functional.conv1d(
        y[0, :].unsqueeze(0).unsqueeze(0),
        torch.ones(1, 1, k),
    ).squeeze()
    for start in torch.where(num_clean < k)[0]:
        start = start.item()
        mask[max(0, start - k) : start + k] = True
    return x[:, mask], y[:, mask]


def get_sample_weights(x, y):
    mask = (x[0, :] < 0.5) & (y[0, :] == 1.0)
    sample_weights = torch.ones(x.size(1))
    sample_weights[mask] = 0.01
    return sample_weights


def generate_data(
    reads_path: str,
    vars: dict[str, pd.DataFrame],
    cbf,
    hist: pd.DataFrame,
    seeds: list[str],
    max_ind: int,
):
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
        x_read, y_read, w_read = [], [], []
        for _, group in read_vars.groupby("group"):
            start = max(group.iloc[0]["Seq_pos"] - k + 1, 0)
            end = group.iloc[-1]["Seq_pos"] + group.iloc[-1]["error_length"]
            start, end = int(start), int(end)
            x = aiedit_torch_extensions.get_model_input(
                record.seq, start, end, seeds, max_ind, int(cbf), hist["error"].tolist()
            )
            y = get_targets(group, end - start, k, max_ind)
            w = get_sample_weights(x, y)
            x_read.append(x)
            y_read.append(y)
            w_read.append(w)
        yield record.id, x_read, y_read, w_read


def train(model, optimizer, read_id, x, y, sample_weights, num_epochs):
    ce_loss = torch.nn.CrossEntropyLoss(reduction="none")
    loss_history = []
    for i in range(num_epochs):
        epoch_loss = 0
        for x_batch, y_batch, w_batch in zip(x, y, sample_weights):
            optimizer.zero_grad()
            y_true = y_batch.permute(1, 0).argmax(dim=1)
            y_pred = model(x_batch).permute(1, 0)
            loss = (ce_loss(y_pred, y_true) * w_batch).mean()
            loss.backward()
            optimizer.step()
            epoch_loss += loss.item()
        epoch_loss /= len(x)
        print(f"[{read_id}] Epoch {i + 1}/{num_epochs}: loss = {epoch_loss:.4f}")
        loss_history.append(epoch_loss)
    return loss_history


def main():
    args = parse_args()
    seeds = read_seeds(args.s)
    hist = read_histogram(args.m)
    model, optimizer = load_model(len(seeds), args.i, args.o)
    print_model_summary(model)
    print("Reading variants file...")
    num_vars, vars = read_vars(args.e)
    print(f"Number of variants = {num_vars}")
    print(f"Number of reads = {len(vars)}")
    print("Reading CBF...")
    cbf = btllib.CountingBloomFilter8(args.b)  # type: ignore
    print(f"CBF FPR = {cbf.get_fpr()}")
    print("Seeds:", *seeds, sep=os.linesep)
    data_generator = generate_data(args.r, vars, cbf, hist, seeds, args.i)
    for read_id, x, y, w in data_generator:
        train(model, optimizer, read_id, x, y, w, args.n)
        state_dict = {
            "model_state_dict": model.state_dict(),
            "optimizer_state_dict": optimizer.state_dict(),
        }
        torch.save(state_dict, args.o)


if __name__ == "__main__":
    main()
