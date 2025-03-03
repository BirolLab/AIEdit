#!/usr/bin/env python3

import argparse
import random
import signal
import sys

import aiedit
import btllib
import numpy as np
import torch
import tqdm
from model import Model


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
        x_seeds = aiedit.ModelInterface.encode_seeds(kmer_model.seeds)
        self._x_seeds = torch.from_numpy(np.array(x_seeds, copy=False))

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
