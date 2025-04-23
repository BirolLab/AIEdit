import argparse
import os
import pathlib
import signal
import time

import btllib

from aiedit import core, utils
from aiedit.model import Model
from aiedit.polisher import Polisher
from aiedit.variants_list import VariantsList


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
    model, _ = Model.from_checkpoint(args.m)
    model.eval()
    model.share_memory()
    end_time = time.perf_counter()
    print(f"DONE ({end_time - start_time:.1f}s)")
    print(f"- Maximum consecutive substitutions: {model._max_mismatches}")
    print(f"- Maximum insertion/deletion size  : {model._max_indels}")
    print()

    file_name = pathlib.Path(args.input_file).stem
    out_prefix = os.path.join(args.o, file_name) + "-aiedit-"
    out_fasta_path = f"{out_prefix}edited.fa"

    polisher = Polisher(model, kmer_model, args.t)

    seq_reader = btllib.SeqReader(args.input_file, btllib.SeqReaderFlag.LONG_MODE)
    seq_writer = btllib.SeqWriter(out_fasta_path)
    variants_list = VariantsList()
    for record in seq_reader:
        seq_name = record.id + (" " + record.comment if record.comment else "")
        print(f"[{seq_name}] Processing started ({len(record.seq):,}bp)")
        start_time = time.perf_counter()
        edits = polisher.polish(record.seq)
        end_time = time.perf_counter()
        elapsed = end_time - start_time
        num_passed = sum(edit[-1] for edit in edits)
        print(f"[{seq_name}] Found {len(edits)} edits in {elapsed:.1f}s")
        variants_list.add(edits, record.seq, record.id, record.comment, len(record.seq))
        edited = core.apply_edits(record.seq, edits)
        print(f"[{seq_name}] Applied {num_passed} passed edits ({len(edited):,}bp)")
        seq_writer.write(record.id, record.comment, edited)
        print(f"[{seq_name}] Edited sequence saved to {out_fasta_path}")

    out_vcf_path = f"{out_prefix}variants.vcf"
    variants_list.save_vcf(out_vcf_path, args.input_file)
    print()
    print(f"Variants saved to {out_vcf_path}")
