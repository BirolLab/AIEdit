#!/usr/bin/env python3

import argparse
import itertools
import random
import signal
import sys

import aiedit
import btllib
import numpy as np
import torch
import torchinfo
import tqdm


class Model(torch.nn.Module):

    def __init__(self, num_seeds: int, model_dim: int):
        super().__init__()
        self._num_seeds = num_seeds
        self.seeds_encoder = torch.nn.GRU(num_seeds, model_dim)
        self.state_encoder = torch.nn.GRU(num_seeds + 1, model_dim)
        self.edits_encoder = torch.nn.GRU(aiedit.ModelInterface.NUM_OUTPUTS, model_dim)
        self.out = torch.nn.Linear(3 * model_dim + 4, aiedit.ModelInterface.NUM_OUTPUTS)

    def forward(self, x_seeds, x_state, x_edits, x_next):
        _, h_seeds = self.seeds_encoder(x_seeds)
        _, h_state = self.state_encoder(x_state)
        _, h_edits = self.edits_encoder(x_edits)
        x_hidden = torch.cat([h_seeds, h_state, h_edits, x_next], dim=-1)
        return self.out(x_hidden)

    def summary(self):
        x_seeds = torch.empty(30, self._num_seeds)
        x_state = torch.empty(32, self._num_seeds + 1)
        x_edits = torch.empty(5, aiedit.ModelInterface.NUM_OUTPUTS)
        x_next = torch.empty(1, 4)
        torchinfo.summary(self, input_data=[x_seeds, x_state, x_edits, x_next])

    @staticmethod
    def encode_seeds(seeds: list[str]):
        x_seeds = torch.empty(size=(len(seeds[0]), len(seeds)))
        indices = itertools.product(range(x_seeds.size(0)), range(x_seeds.size(1)))
        for i, j in indices:
            x_seeds[i][j] = float(seeds[j][i])
        return x_seeds


def epsilon_greedy(y_model: torch.Tensor, epsilon: float) -> int:
    if random.random() < epsilon:
        return y_model.argmax().item()
    else:
        return random.randint(0, aiedit.ModelInterface.NUM_OUTPUTS - 1)


class ModelTrainer:

    def __init__(
        self,
        model: Model,
        kmer_model,
        max_edits: int,
        gamma: float = 0.99,
        epsilon: float = 0.75,
    ):
        self._model = model
        self._kmer_model = kmer_model
        self._max_edits = max_edits
        self._gamma = gamma
        self._epsilon = epsilon
        self._optim = torch.optim.AdamW(model.parameters())
        self._x_seeds = model.encode_seeds(kmer_model.seeds)

    def train(self, seq, start, end):
        env = aiedit.ModelInterface(seq, start, end, self._max_edits, self._kmer_model)
        x_signature = torch.from_numpy(np.array(env.get_signature(), copy=False))
        initial_value = x_signature.mean()
        x_edits = torch.zeros(size=(self._max_edits, aiedit.ModelInterface.NUM_OUTPUTS))
        i_edit = 0
        total_loss, final_reward = 0.0, torch.zeros([])
        while not env.is_terminated():
            x_next = torch.tensor(env.get_next_probs()).unsqueeze(0)
            y_model = self._model(self._x_seeds, x_signature, x_edits.clone(), x_next)
            y_model.squeeze_(0)
            i_action = epsilon_greedy(y_model, self._epsilon)
            x_edits[i_edit, i_action] = 1.0
            i_edit += 1
            env(i_action)
            x_state = np.array(env.get_signature(), copy=False)
            if not x_state[:, 0].any():
                env.terminate()
                final_value = x_state.mean() if x_state.shape[0] > 0 else 0
                edits_reward = env.num_edits_left / self._max_edits
                final_reward = final_value - initial_value + edits_reward
            target = final_reward.clone()
            if not env.is_terminated:
                with torch.no_grad():
                    x1_next = torch.tensor(env.get_next_probs()).unsqueeze(0)
                    y_next = self._model(self._x_seeds, x_signature, x_edits, x1_next)
                    target += self._gamma * y_next.amax(dim=-1)
            loss = torch.nn.functional.huber_loss(y_model[i_action], target)
            self._optim.zero_grad()
            loss.backward()
            self._optim.step()
            total_loss += loss.item()
        return total_loss, final_reward.item()


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("input_file", help="path to input assembly")
    parser.add_argument("-c", help="path to counting bloom filter", required=True)
    parser.add_argument("-p", help="path to k-mer histogram model", required=True)
    parser.add_argument("-s", help="path to spaced seeds file", required=True)
    parser.add_argument("-e", help="maximum number of edits", type=int, default=5)
    return parser.parse_args()


def main():
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    args = parse_args()
    print("Loading k-mer model")
    kmer_model = aiedit.KmerModel(args.c, args.p, args.s, 0.5)
    print("Done")
    model = Model(len(kmer_model.seeds), 32)
    model.summary()
    trainer = ModelTrainer(model, kmer_model, args.e)
    pbar = tqdm.tqdm(file=sys.stdout, mininterval=5)
    seq_reader = btllib.SeqReader(args.input_file, btllib.SeqReaderFlag.LONG_MODE)
    for record in seq_reader:
        if len(record.seq) < 2 * kmer_model.kmer_size + args.e:
            continue
        erf = aiedit.EditRegionFinder(record.seq, kmer_model)
        for region in erf:
            if args.e <= region[1] - region[0] <= args.e + kmer_model.kmer_size:
                loss, final_reward = trainer.train(record.seq, *region)
                pbar.set_postfix_str(f"loss={loss:.4f}, reward={final_reward:.4f}")
                pbar.update()


if __name__ == "__main__":
    main()
