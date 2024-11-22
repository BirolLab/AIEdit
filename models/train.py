import argparse
import os
import re
import warnings

import btllib
import numpy as np
import pandas as pd
from keras.layers import Conv1D, Input
from keras.models import Sequential, load_model


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


def generate_data(
    seq_path: str,
    vars_path: str,
    cbf_path: str,
    hist_path: str,
    seeds: list[str],
    max_ind: int,
):
    print("reading variations file...")
    vars = pd.read_csv(vars_path, delimiter=r"\s+", nrows=2000)
    print(f"number of vars: {len(vars)}")
    print("grouping vars based on reads...")
    vars = {seq_name: data for seq_name, data in vars.groupby("Seq_name")}
    print(f"num reads = {len(vars)}")
    print("reading cbf...")
    cbf = btllib.CountingBloomFilter8(cbf_path)  # type: ignore
    num_hashes = cbf.get_hash_num()
    print(f"cbf fpr = {cbf.get_fpr()}")
    hist = pd.read_csv(hist_path, delimiter=r"\s+", index_col=0)
    hist.loc[0] = [0, 1, 0, 0]
    norm_cols = ["error", "heterozygous", "homozygous"]
    row_sum = hist[norm_cols].sum(axis=1)
    hist[norm_cols] = hist[norm_cols].div(row_sum, axis=0)
    sr = btllib.SeqReader(seq_path, btllib.SeqReaderFlag.LONG_MODE)  # type: ignore
    print("seeds:", *seeds, sep=os.linesep)
    assert len(set(map(len, seeds))) == 1, "seeds should be the same length"
    k = len(seeds[0])
    for record in sr:
        match = re.search(r"F_(\d+)_\d+_(\d+)", record.id)
        if not match:
            warnings.warn(f"invalid read id: {record.id}")
            continue
        if record.id not in vars:
            warnings.warn(f"read has no errors: {record.id}")
            continue
        head = int(match.group(1))
        tail = int(match.group(2))
        x = np.zeros(shape=(1, len(record.seq) - k + 1, len(seeds) + max_ind))
        y = np.zeros(shape=(1, len(record.seq) - k + 1, max_ind + 3))
        y[0, :, 0] = 1.0
        for i_seed, seed in enumerate(seeds):
            svec = btllib.parse_seeds([seed])  # type: ignore
            nh = btllib.SeedNtHash(record.seq, svec, num_hashes, k, head)  # type: ignore
            while nh.roll() and nh.get_pos() < len(record.seq) - tail:
                err_prob = hist.loc[cbf.contains(nh.hashes())]["error"]
                x[0, nh.get_pos() - head, i_seed] = err_prob
        sh = SkipHash(record.seq, num_hashes, k, max_ind, head)
        while sh.roll() and sh.get_pos() < len(record.seq) - tail:
            if sh.get_pos() - head < k // 2:
                continue
            for i in range(len(sh.hashes())):
                err_prob = hist.loc[cbf.contains(sh.hashes()[i])]["error"]
                x[0, sh.get_pos() - head - k // 2, i + len(seeds)] = err_prob
        x[0, -k // 2 :, len(seeds) :] = 1.0
        pos_diff = 0
        for _, row in vars[record.id].sort_values("Seq_pos").iterrows():
            seq_pos = row["Seq_pos"] + pos_diff - k + 1
            if row["error_type"] == "mis":
                y[0, seq_pos : seq_pos + row["error_length"], 0] = 0.0
                y[0, seq_pos : seq_pos + row["error_length"], 1] = 1.0
            elif row["error_type"] == "ins":
                y[0, seq_pos : seq_pos + row["error_length"], 0] = 0.0
                y[0, seq_pos : seq_pos + row["error_length"], 2] = 1.0
                pos_diff += row["error_length"]
            elif row["error_type"] == "del":
                num_ins = min(row["error_length"], y.shape[-1] - 3)
                y[0, seq_pos - 1, 0] = 0.0
                y[0, seq_pos - 1, num_ins + 2] = 1.0
                pos_diff -= row["error_length"]
        np.set_printoptions(precision=3)
        np.set_printoptions(linewidth=np.inf)
        for i in range(x.shape[1]):
            print(i, x[0, i], y[0, i])
        return x, y
        # yield record.id, x, y


def make_model(num_seeds: int, max_ind: int):
    model = Sequential(
        [
            Input(shape=(None, num_seeds)),
            Conv1D(64, kernel_size=30, padding="same", activation="relu"),
            Conv1D(128, kernel_size=30, padding="same", activation="relu"),
            Conv1D(64, kernel_size=30, padding="same", activation="relu"),
            Conv1D(max_ind + 3, kernel_size=30, padding="same", activation="softmax"),
        ]
    )
    return model


def main():
    args = parse_args()
    with open(args.s) as fp:
        seeds = list(map(str.strip, fp.readlines()))
    generate_data(args.r, args.e, args.b, args.m, seeds, args.i)
    return
    model = None
    if os.path.isfile(args.o):
        model = load_model(args.o)
    if not model:
        model = make_model(len(seeds), args.i)
    model.compile(optimizer="adam", loss="categorical_crossentropy")
    model.summary()
    for read_id, x, y in generate_data(args.r, args.e, args.b, args.m, seeds, args.i):
        print(read_id)
        model.fit(x, y, epochs=args.n)
        model.save(args.o)


if __name__ == "__main__":
    main()
