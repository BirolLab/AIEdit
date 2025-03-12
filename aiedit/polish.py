import argparse
import os
import pathlib
import signal
import time

import btllib
import torch

from aiedit import core
from aiedit.model import Model
from aiedit.sequence_polisher import SequencePolisher


def add_subparser(subparsers: argparse._SubParsersAction) -> None:
    parser: argparse.ArgumentParser = subparsers.add_parser("polish")
    parser.add_argument("input_file", help="path to assembly file")
    parser.add_argument("-c", help="path to counting bloom filter", required=True)
    parser.add_argument("-k", help="path to k-mer histogram model", required=True)
    parser.add_argument("-s", help="path to spaced seeds file", required=True)
    parser.add_argument("-m", help="path to pretrained model", required=True)
    parser.add_argument("-p", help="hit probability threshold", type=float, default=0.5)
    parser.add_argument("-y", help="maximum number of edits", type=int, default=5)
    parser.add_argument("-t", help="number of threads", type=int, default=1)
    parser.add_argument("--contigs", action="store_true", help="polishing contigs")
    parser.add_argument("-o", help="output directory path", default=".")
    parser.set_defaults(func=main)


def main(args):
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    start_time = time.perf_counter()
    print("Loading k-mer model... ", end="", flush=True)
    kmer_model = core.KmerModel(args.c, args.k, args.s)
    end_time = time.perf_counter()
    print(f"DONE ({end_time - start_time:.1f}s)")
    print("Loading edit model... ", end="", flush=True)
    start_time = time.perf_counter()
    checkpoint = torch.load(args.m, weights_only=True)
    model = EditModel(checkpoint["num_seeds"].item(), checkpoint["model_dim"].item(), 0)
    model.load_state_dict(checkpoint["model"])
    model.eval()
    end_time = time.perf_counter()
    print(f"DONE ({end_time - start_time:.1f}s)")
    file_name = pathlib.Path(args.input_file).stem
    out_prefix = os.path.join(args.o, file_name) + "_"
    print(f"Output prefix: {out_prefix}")
    polisher = SequencePolisher(
        kmer_model, model, args.p, args.y, args.t, args.contigs, out_prefix
    )
    seq_reader = btllib.SeqReader(args.input_file, btllib.SeqReaderFlag.LONG_MODE)
    for record in seq_reader:
        polisher.queue_sequence(record.id, record.seq, record.comment)
    polisher.close()
