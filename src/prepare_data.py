import argparse
import os
import re
import warnings

import aiedit_torch_extensions as ext
import btllib
import pandas as pd
import torch
import tqdm


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("-r", help="path to sequences", required=True)
    parser.add_argument("-e", help="path to variants list file", required=True)
    parser.add_argument("-b", help="path to counting bloom filter", required=True)
    parser.add_argument("-m", help="path to histogram model file", required=True)
    parser.add_argument("-s", help="path to seeds file", required=True)
    parser.add_argument("-i", help="maximum indel length", type=int, default=10)
    parser.add_argument("-o", help="path to store dataset", default="data.pt")
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
    max_indels: int,
):
    data = []
    probs = hist["error"].tolist()
    sr = btllib.SeqReader(reads_path, btllib.SeqReaderFlag.LONG_MODE)  # type: ignore
    for record in tqdm.tqdm(sr, total=len(vars), unit="reads", desc="Generating data"):
        match = re.search(r"F_(\d+)_\d+_\d+", record.id)
        if not match:
            warnings.warn(f"invalid read id: {record.id}")
            continue
        if record.id not in vars:
            warnings.warn(f"read has no errors: {record.id}")
            continue
        seq, k = record.seq, len(seeds[0])
        read_vars = vars[record.id].sort_values("Seq_pos")
        read_vars["Seq_pos"] += int(match.group(1))
        pos_ends = (read_vars["Seq_pos"] + read_vars["error_length"]).shift().fillna(0)
        read_vars["group"] = (read_vars["Seq_pos"] - pos_ends > 2 * k).cumsum()
        pos_diff = 0
        for _, group in read_vars.groupby("group"):
            start = max(group.iloc[0]["Seq_pos"] - k + 1, 0) + pos_diff
            end = group.iloc[-1]["Seq_pos"] + group.iloc[-1]["error_length"] + pos_diff
            x = ext.get_model_input(seq, start, end, seeds, max_indels, int(cbf), probs)
            y, pos_diff = get_targets(group, pos_diff)
            if validiate_sample(x, y, len(seeds)):
                data.append((x, y))
    return data


def main():
    args = parse_args()
    seeds = read_seeds(args.s)
    hist = read_histogram(args.m)
    print("Loading variants file...")
    num_vars, vars = read_vars(args.e)
    print(f"Number of variants: {num_vars}")
    print(f"Number of reads: {len(vars)}")
    print("Loading CBF...")
    cbf = btllib.CountingBloomFilter8(args.b)  # type: ignore
    print(f"CBF FPR: {cbf.get_fpr()}")
    print("Seeds:", *seeds, sep=os.linesep)
    data = generate_data(args.r, vars, cbf, hist, seeds, args.i)
    print(f"Gathered {len(data)} patterns")
    print("Saving dataset...")
    torch.save({"seeds": seeds, "max_indels": args.i, "data": data}, args.o)
    print(f"Data available in {args.o}")


if __name__ == "__main__":
    main()
