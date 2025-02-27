#!/usr/bin/env python3

import argparse
import enum
import signal
import itertools

import torchinfo

import aiedit
import btllib
import torch
import numpy as np
import tqdm


class Model(torch.nn.Module):

    EDITS = [
        (aiedit.EditType.NONE, ""),
        (aiedit.EditType.SUBSTITUTE, "A"),
        (aiedit.EditType.SUBSTITUTE, "C"),
        (aiedit.EditType.SUBSTITUTE, "G"),
        (aiedit.EditType.SUBSTITUTE, "T"),
        (aiedit.EditType.INSERT, "A"),
        (aiedit.EditType.INSERT, "C"),
        (aiedit.EditType.INSERT, "G"),
        (aiedit.EditType.INSERT, "T"),
        (aiedit.EditType.DELETE, ""),
    ]

    def __init__(self, num_seeds: int, model_dim: int):
        super().__init__()
        self._num_seeds = num_seeds
        self.seeds_encoder = torch.nn.GRU(num_seeds, model_dim)
        self.state_encoder = torch.nn.GRU(num_seeds, model_dim)
        self.out = torch.nn.Linear(2 * model_dim + 4, len(Model.EDITS) + 1)

    def forward(self, x_seeds, x_state, x_next):
        _, x_seeds = self.seeds_encoder(x_seeds)
        _, x_state = self.state_encoder(x_state)
        x_hidden = torch.cat([x_seeds, x_state, x_next], -1)
        return self.out(x_hidden)

    def summary(self):
        x_seeds = torch.empty(30, self._num_seeds)
        x_state = torch.empty(32, self._num_seeds)
        x_next = torch.empty(1, 4)
        torchinfo.summary(self, input_data=[x_seeds, x_state, x_next])

    @staticmethod
    def encode_seeds(seeds: list[str]):
        x_seeds = torch.empty(size=(len(seeds[0]), len(seeds)))
        indices = itertools.product(range(x_seeds.size(0)), range(x_seeds.size(1)))
        for i, j in indices:
            x_seeds[i][j] = float(seeds[j][i])
        return x_seeds


class ModelTrainer:

    def __init__(self, model: Model, kmer_model, max_edits: int, gamma: float = 0.99):
        self._model = model
        self._kmer_model = kmer_model
        self._max_edits = max_edits
        self._gamma = gamma
        self._optim = torch.optim.SGD(model.parameters())
        self._x_seeds = model.encode_seeds(kmer_model.seeds)

    def train(self, seq, start, end):
        env = aiedit.Environment(seq, start, end, self._max_edits, self._kmer_model)
        loss, final_reward = 0, 0
        while not env.is_terminated():
            x_state = np.array(env.get_state().signature, copy=False)
            x_next = torch.tensor(env.get_state().next_probs).unsqueeze(0)
            x_model = (self._x_seeds, torch.from_numpy(x_state), x_next)
            y_model = self._model(*x_model)
            i_action = y_model.argmax(dim=-1)
            final_reward = env.act(*Model.EDITS[i_action])
            target = final_reward
            if not env.is_terminated():
                with torch.no_grad():
                    x1_state = np.array(env.get_state().signature, copy=False)
                    x1_next = torch.tensor(env.get_state().next_probs).unsqueeze(0)
                    x1_model = (self._x_seeds, torch.from_numpy(x1_state), x1_next)
                    target += self._gamma * self._model(*x1_model).max(dim=-1)
            loss += torch.nn.functional.huber_loss(y_model[i_action], target)
        self._optim.zero_grad()
        loss.backward()
        self._optim.step()
        return loss.item(), final_reward


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("input_file", help="path to input assembly")
    parser.add_argument("-c", help="path to counting bloom filter", required=True)
    parser.add_argument("-p", help="path to k-mer histogram model", required=True)
    parser.add_argument("-s", help="path to spaced seeds file", required=True)
    parser.add_argument("-e", help="maximum number of edits", type=int, default=10)
    return parser.parse_args()


def main():
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    args = parse_args()
    print("Loading k-mer model")
    kmer_model = aiedit.KmerModel(args.c, args.p, args.s)
    print("Done")
    model = Model(len(kmer_model.seeds))
    model.summary()
    trainer = ModelTrainer(model, kmer_model, args.e)
    num_sequences = 0
    seq_reader = btllib.SeqReader(args.input_file, btllib.SeqReaderFlag.LONG_MODE)
    for record in seq_reader:
        seq_editor = aiedit.Editor(record.seq, kmer_model)
        region = seq_editor.get_next_region()
        while region:
            if region[1] - region[0] <= 35:
                loss, final_reward = trainer.train(record.seq, *region)
                print(region, loss, final_reward)
            region = seq_editor.get_next_region()
        num_sequences += 1


if __name__ == "__main__":
    main()
