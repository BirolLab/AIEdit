import argparse
import signal
import time

import btllib
import torch

from aiedit import core
from aiedit.edit_model import EditModel
from aiedit.supervised_trainer import SupervisedTrainer


def add_subparser(subparsers: argparse._SubParsersAction) -> None:
    parser: argparse.ArgumentParser = subparsers.add_parser("init")
    parser.add_argument("input_file", help="path to reference genome")
    parser.add_argument("-c", help="path to counting bloom filter", required=True)
    parser.add_argument("-k", help="path to k-mer histogram model", required=True)
    parser.add_argument("-s", help="path to spaced seeds file", required=True)
    parser.add_argument("-p", help="hit probability threshold", type=float, default=0.5)
    parser.add_argument("-y", help="maximum number of edits", type=int, default=5)
    parser.add_argument("-d", help="model dimensionality", type=int, default=32)
    parser.add_argument("-e", help="number of epochs", type=float, default=100)
    parser.add_argument("-f", help="checkpoint save frequency", type=int, default=1000)
    parser.add_argument("-o", help="checkpoint path", default="checkpoint.pt")
    parser.set_defaults(func=main)


def main(args):
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    print("Loading k-mer model... ", end="", flush=True)
    start_time = time.perf_counter()
    kmer_model = core.KmerModel(args.c, args.k, args.s)
    end_time = time.perf_counter()
    print(f"DONE ({end_time - start_time:.1f}s)")
    model = EditModel(len(kmer_model.seeds), args.d, args.e)
    optim = torch.optim.AdamW(model.parameters(), 0.01)
    model.train()
    model.summary()
    trainer = SupervisedTrainer(model, optim, kmer_model, args.p, args.y)
    seq_reader = btllib.SeqReader(args.input_file, btllib.SeqReaderFlag.LONG_MODE)
    for record in seq_reader:
        trainer.train(record.seq)
