#!/usr/bin/env python3

import argparse
import signal

import aiedit
import btllib
import torch
import tqdm


class Model(torch.nn.Module):

    def __init__(self):
        super().__init__()

    def forward(self, x_state):
        pass

    def summary(self):
        pass


class ModelTrainer:

    def __init__(self, model: Model, kmer_model):
        self._model = model
        self._kmer_model = kmer_model
        # self._optim = torch.optim.SGD(model.parameters())

    def train(self, seq, start, end):
        env = aiedit.Environment(seq, start, end, self._kmer_model)
        print(torch.frombuffer(env.get_state().signature, dtype=torch.float))


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("input_file", help="path to input assembly")
    parser.add_argument("-c", help="path to counting bloom filter", required=True)
    parser.add_argument("-p", help="path to k-mer histogram model", required=True)
    parser.add_argument("-s", help="path to spaced seeds file", required=True)
    return parser.parse_args()


def main():
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    args = parse_args()
    model = Model()
    print("Loading k-mer model")
    kmer_model = aiedit.KmerModel(args.c, args.p, args.s)
    print("Done")
    trainer = ModelTrainer(model, kmer_model)
    num_sequences = 0
    seq_reader = btllib.SeqReader(args.input_file, btllib.SeqReaderFlag.LONG_MODE)
    for record in seq_reader:
        seq_editor = aiedit.Editor(record.seq, kmer_model)
        region = seq_editor.get_next_region()
        while region:
            if region[1] - region[0] > 35:
                region = seq_editor.get_next_region()
                continue
            print(region)
            trainer.train(record.seq, *region)
            return
        num_sequences += 1


if __name__ == "__main__":
    main()
