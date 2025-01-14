import argparse
import json
import math
import os
import random
import re

import aiedit_torch_extensions as ext  # type: ignore
import btllib
import pandas as pd
import torch
import torchinfo
import tqdm


class Model(torch.nn.Module):

    def __init__(self, num_seeds: int, max_indels: int, hidden_size: int):
        super().__init__()
        probs_dim = num_seeds + 2 * max_indels + 1
        self._input_dims = [(35, num_seeds), (40, probs_dim), (3, 4)]
        self.num_seeds = torch.jit.Attribute(num_seeds, int)
        self.max_indels = torch.jit.Attribute(max_indels, int)
        self.seeds_encoder = torch.nn.GRU(num_seeds, hidden_size)
        self.probs_encoder = torch.nn.GRU(probs_dim, hidden_size)
        self.state_encoder = torch.nn.GRU(4, hidden_size)
        self.q_out = torch.nn.Linear(3 * hidden_size, 5)

    @torch.jit.ignore
    def summary(self):
        torchinfo.summary(self, self._input_dims, depth=1)

    def forward(self, x_seeds, x_probs, x_state):
        _, h_seeds = self.seeds_encoder(x_seeds)
        _, h_probs = self.probs_encoder(x_probs)
        _, h_state = self.state_encoder(x_state)
        return self.q_out(torch.cat([h_seeds, h_probs, h_state], dim=-1))


def apply_edit(seq, pos, edit):
    return seq, pos


class ModelTrainer:

    def __init__(
        self,
        model: Model,
        optimizer: torch.optim.Optimizer,
        seeds: list[str],
        cbf,
        probs: list[float],
        threshold: float,
        max_edits: int,
        gamma: float,
    ):
        self._model = model
        self._optim = optimizer
        self._seeds = seeds
        self._cbf = cbf
        self._probs = probs
        self._threshold = threshold
        self._max_edits = max_edits
        self._gamma = gamma
        self._x_seeds = ext.encode_seeds(seeds)
        self._probs_args = (seeds, model.max_indels.value, int(cbf), probs)
        self.score_history = []

    def train(self, seq: str, start: int, end: int):
        x_probs = ext.get_model_input(seq, start, end, *self._probs_args)
        mask = x_probs[:, 1] < self._threshold
        diffs = torch.diff(mask.to(torch.int8), prepend=torch.tensor([0]))
        starts = (diffs == 1).nonzero(as_tuple=True)[0]
        ends = (diffs == -1).nonzero(as_tuple=True)[0]
        pairs = zip(starts, ends)
        filtered_pairs = filter(lambda i, j: (j - i).item() < self._max_edits, pairs)
        for i, j in filtered_pairs:

            sum_rewards, total_loss = self._simulate_edits(seq[i:j], x_probs)

    def _simulate_edits(self, seq: str, x_probs: torch.Tensor):
        sum_rewards, total_loss = 0, 0
        x_steps = torch.zeros(1, 4)
        k = len(self._seeds[0])
        for _ in range(self._max_edits):
            y_model = self._model(self._x_seeds, x_probs, x_steps)
            action = y_model.argmax(dim=-1)
            if action == 1:
                pass
            elif action == 2:
                pass
            elif action == 3:
                pass
            elif action == 4:
                pass
            step = torch.zeros(4)
            step[action] = 1.0
            x_steps = torch.cat([x_steps, step])
            new_probs = ext.get_model_input(seq, 0, len(seq) - k, *self._probs_args)
            rewards = new_probs - x_probs
            rewards[:, len(self._seeds) + 1 :] *= -1.0
            reward = rewards.mean()
            sum_rewards += reward
            next_q = self._model(self._x_seeds, new_probs, x_steps).max(dim=-1).detach()
            q_target = reward + self._gamma * next_q
            total_loss += torch.nn.functional.huber_loss(y_model[action], q_target)
        num_edits = x_steps.size(0) - 1
        return sum_rewards / num_edits, total_loss / num_edits


def parse_args():
    default_t = torch.get_num_threads()
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("-r", help="path to sequences", required=True)
    parser.add_argument("-c", help="path to counting bloom filter", required=True)
    parser.add_argument("-h", help="path to histogram model file", required=True)
    parser.add_argument("-s", help="path to seeds file", required=True)
    parser.add_argument("-i", help="maximum indel length", type=int, default=10)
    parser.add_argument("-d", help="hidden states size", type=int, default=32)
    parser.add_argument("-y", help="maximum output edits", type=int, default=20)
    parser.add_argument("-p", help="probability threshold", type=float, default=0.5)
    parser.add_argument("-g", help="training gamma", type=float, default=0.9)
    parser.add_argument("-t", help="number of threads", type=int, default=default_t)
    parser.add_argument("-o", help="output files prefix", default="model")
    return parser.parse_args()


def read_seeds(path: str) -> list[str]:
    with open(path) as file:
        seeds = [line.strip() for line in file]
    assert len(set(map(len, seeds))) == 1, "seeds should be the same length"
    return seeds


def read_histogram(path: str) -> pd.DataFrame:
    hist = pd.read_csv(path, delimiter=r"\s+", index_col=0)
    hist.loc[0] = [0, 1, 0, 0]
    norm_cols = ["error", "heterozygous", "homozygous"]
    row_sum = hist[norm_cols].sum(axis=1)
    hist[norm_cols] = hist[norm_cols].div(row_sum, axis=0)
    return hist["error"].tolist()


def main():
    args = parse_args()
    seeds = read_seeds(args.s)
    probs = read_histogram(args.h)
    model = Model(len(seeds), args.i, args.d)
    optimizer = torch.optim.AdamW(model.parameters())
    model.summary()
    print("Loading CBF...")
    cbf = btllib.CountingBloomFilter8(args.c)  # type: ignore
    print(f"- Size (bytes): {cbf.get_bytes():,}")
    print(f"- False positive rate: {cbf.get_fpr():.6f}")
    print(f"Maximum indel length: {args.d}")
    print(f"Number of seeds: {len(seeds)}")
    print(f"Using {torch.get_num_threads()} threads")
    trainer = ModelTrainer(model, optimizer, seeds, cbf, probs, args.p, args.y, args.g)
    pbar = tqdm.tqdm(desc="Training", disable=True)
    postfix = {"num_reads": 0, "reward": 0}
    sr = btllib.SeqReader(args.r, btllib.SeqReaderFlag.LONG_MODE)  # type: ignore
    for record in sr:
        print(record.id)
        match = re.search(r"[FR]_(\d+)_\d+_(\d+)", record.id)
        if not match:
            pbar.write(f"invalid read id: {record.id}")
            continue
        head, tail = int(match.group(1)), int(match.group(2)) - len(seeds[0])
        while trainer.train(record.seq, head, len(record.seq) - tail):
            continue
        return
        for reward in trainer.train(record.seq, head, tail):
            postfix["reward"] = reward
            pbar.set_postfix(postfix)
            pbar.update()
        postfix["num_reads"] += 1
        pbar.set_postfix(postfix)
        break


if __name__ == "__main__":
    main()
