import argparse
import os
import pathlib
import signal
import time

import btllib

from aiedit import core, utils
from aiedit.vcf_writer import VCFWriter


def add_subparser(subparsers: argparse._SubParsersAction) -> None:
    parser: argparse.ArgumentParser = subparsers.add_parser("polish")
    parser.add_argument("input_file", help="path to assembly file")
    parser.add_argument("-b", help="path to bf/cbf", required=True)
    parser.add_argument("-k", help="path to k-mer histogram model (if using cbf)")
    parser.add_argument("-s", help="path to spaced seeds file (if using cbf)")
    parser.add_argument("-m", help="path to pretrained model", required=True)
    parser.add_argument("-p", help="hit probability threshold", type=float, default=0.5)
    parser.add_argument("-t", help="number of threads", type=int, default=1)
    parser.add_argument("-o", help="output directory path", default=".")
    parser.set_defaults(func=main)


def main(args):
    signal.signal(signal.SIGINT, signal.SIG_DFL)

    print("Loading bloom filter... ", end="", flush=True)
    start_time = time.perf_counter()
    kmer_model = core.CBFKmerModel(args.b, args.k or "", utils.load_seeds(args.s))
    end_time = time.perf_counter()
    print(f"DONE ({end_time - start_time:.1f}s)")
    print(f"- Size (bytes)   : {kmer_model.get_size():,}")
    print(f"- Number of seeds: {len(kmer_model.get_seeds())}")
    print(f"- K-mer size     : {kmer_model.get_kmer_size()}")
    print()

    print("Loading edit model... ", end="", flush=True)
    start_time = time.perf_counter()
    polisher = core.Polisher(args.m, kmer_model, args.t, args.p)
    end_time = time.perf_counter()
    print(f"DONE ({end_time - start_time:.1f}s)")
    print(f"- Maximum consecutive substitutions: {polisher.get_max_mismatches()}")
    print(f"- Maximum insertion/deletion size  : {polisher.get_max_indels()}")
    print()

    file_name = pathlib.Path(args.input_file).stem
    out_prefix = os.path.join(args.o, file_name) + "-aiedit-"
    out_fasta_path = f"{out_prefix}edited.fa"
    out_vcf_path = f"{out_prefix}variants.vcf"

    seq_reader = btllib.SeqReader(args.input_file, btllib.SeqReaderFlag.LONG_MODE)
    seq_writer = btllib.SeqWriter(out_fasta_path)
    vcf_writer = VCFWriter(args.input_file, args.p)
    for record in seq_reader:
        print(f"[{record.id}] Processing started ({len(record.seq):,}bp)")
        start_time = time.perf_counter()
        edits = polisher.polish(record.seq)
        end_time = time.perf_counter()
        elapsed = end_time - start_time
        print(f"[{record.id}] Found {len(edits)} edits in {elapsed:.1f}s")
        vcf_writer.add(record.id, record.comment, record.seq, edits)
        edited = edits.apply(record.seq)
        num_passed = edits.get_num_passed()
        print(f"[{record.id}] Applied {num_passed} passed edits ({len(edited):,}bp)")
        seq_writer.write(record.id, record.comment, edited)
        print(f"[{record.id}] Edited sequence saved to {out_fasta_path}")

    vcf_writer.write(out_vcf_path)
    print()
    print(f"Variants saved to {out_vcf_path}")
