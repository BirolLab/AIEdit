import argparse
import os
import re
import warnings

import btllib
import numpy as np
import pandas as pd
import torch
import torchinfo


def extend_hashes(fwd_hash, rev_hash, k, h):
    hash_array = [0] * h
    hash_array[0] = (fwd_hash + rev_hash) % (2**64)
    for i in range(1, h):
        t_val = (hash_array[0] * (i ^ k * btllib.MULTISEED) % (2**64)) % (2**64)  # type: ignore
        t_val ^= (t_val >> btllib.MULTISHIFT) % (2**64)  # type: ignore
        hash_array[i] = t_val % (2**64)
    return hash_array


class SkipHash:

    def __init__(self, seq, h, k, g, p0=0) -> None:
        self._h = h
        self.__hashers = [btllib.NtHash(seq, 1, k // 2)]  # type: ignore
        for _ in range(p0):
            self.__hashers[0].roll()
        for i in range(g):
            pos = k // 2 + i + p0 + 1
            self.__hashers.append(btllib.NtHash(seq, 1, k - k // 2))  # type: ignore
            for _ in range(pos):
                self.__hashers[-1].roll()

    def roll(self):
        can_roll = True
        for i in range(len(self.__hashers)):
            can_roll = self.__hashers[i].roll()
        return can_roll

    def get_pos(self):
        return self.__hashers[0].get_pos()

    def hashes(self):
        del_hashes = []
        k = self.__hashers[0].get_k() + self.__hashers[1].get_k()
        f_0 = self.__hashers[0].get_forward_hash()
        r_0 = self.__hashers[0].get_reverse_hash()
        for i in range(1, len(self.__hashers)):
            f_i = btllib.srol(f_0, self.__hashers[i].get_k()) ^ self.__hashers[i].get_forward_hash()  # type: ignore
            r_i = r_0 ^ btllib.srol(self.__hashers[i].get_reverse_hash(), self.__hashers[0].get_k())  # type: ignore
            del_hashes.append(extend_hashes(f_i, r_i, k, self._h))
        return del_hashes


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
        torch.nn.Conv1d(num_seeds + max_ind, 64, 7, padding="same"),
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


def get_inputs(
    seq: str,
    head: int,
    tail: int,
    seeds: list[str],
    max_indels: int,
    cbf,
    hist: pd.DataFrame,
):
    k = len(seeds[0])
    num_hashes = cbf.get_hash_num()
    x = torch.zeros(len(seeds) + max_indels, len(seq) - k + 1)
    for i_seed, seed in enumerate(seeds):
        svec = btllib.parse_seeds([seed])  # type: ignore
        nh = btllib.SeedNtHash(seq, svec, num_hashes, k, head)  # type: ignore
        while nh.roll() and nh.get_pos() < len(seq) - tail:
            err_prob = hist.loc[cbf.contains(nh.hashes())]["error"]
            x[i_seed, nh.get_pos() - head] = err_prob
    sh = SkipHash(seq, num_hashes, k, max_indels, head)
    while sh.roll() and sh.get_pos() < len(seq) - tail:
        if sh.get_pos() - head < k // 2:
            continue
        for i in range(len(sh.hashes())):
            err_prob = hist.loc[cbf.contains(sh.hashes()[i])]["error"]
            x[i + len(seeds), sh.get_pos() - head - k // 2] = err_prob
    x[len(seeds) :, -k // 2 :] = 1.0
    return x


def get_targets(vars: pd.DataFrame, seq_len: int, k: int, max_indels):
    y = torch.zeros(max_indels + 3, seq_len - k + 1)
    y[0, :] = 1.0
    pos_diff = 0
    for _, row in vars.sort_values("Seq_pos").iterrows():
        seq_pos = row["Seq_pos"] + pos_diff - k + 1
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
    sample_weights = torch.ones(x.size(1))
    mask = (x[0, :] < 0.5) & (y[0, :] == 1.0)
    sample_weights[mask] = 1 - y[0, :].sum() / y.size(1)
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
        match = re.search(r"F_(\d+)_\d+_(\d+)", record.id)
        if not match:
            warnings.warn(f"invalid read id: {record.id}")
            continue
        if record.id not in vars:
            warnings.warn(f"read has no errors: {record.id}")
            continue
        head = int(match.group(1))
        tail = int(match.group(2))
        x = get_inputs(record.seq, head, tail, seeds, max_ind, cbf, hist)
        y = get_targets(vars[record.id], len(record.seq), len(seeds[0]), max_ind)
        x, y = filter_samples(x, y, len(seeds[0]))
        sample_weights = get_sample_weights(x, y)
        yield record.id, x, y, sample_weights


def train(model, optimizer, read_id, x, y_true, sample_weights, num_epochs):
    ce_loss = torch.nn.CrossEntropyLoss(reduction="none")
    y_true = y_true.permute(1, 0).argmax(dim=1)
    loss_history = []
    for i in range(num_epochs):
        optimizer.zero_grad()
        y_pred = model(x).permute(1, 0)
        loss = (ce_loss(y_pred, y_true) * sample_weights).mean()
        loss.backward()
        optimizer.step()
        print(f"[{read_id}] Epoch {i + 1}/{num_epochs}: loss = {loss.item():.4f}")
        loss_history.append(loss.item())
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
