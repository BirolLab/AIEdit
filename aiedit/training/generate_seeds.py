import argparse
import os

from aiedit import core


def add_subparser(subparsers: argparse._SubParsersAction) -> None:
    parser: argparse.ArgumentParser = subparsers.add_parser("generate_seeds")
    parser.add_argument(
        "-n", "--num-seeds", help="number of seeds", type=int, required=True
    )
    parser.add_argument("-k", "--kmer-size", help="kmer size", type=int, required=True)
    parser.add_argument(
        "-m", "--max-mismatches", help="mismatch window size", type=int, default=5
    )
    parser.add_argument(
        "-i", "--max-indels", help="indel window size", type=int, default=10
    )
    parser.add_argument(
        "-t", "--num-threads", help="number of parallel threads", type=int, default=1
    )
    parser.add_argument(
        "-o", "--out-file", help="path to store spaced seeds file", required=True
    )
    parser.set_defaults(func=main)


def main(args):
    seed_generator = core.SeedGenerator(100, 100, 0.5)
    print("Generating seeds... ", end="")
    seeds = seed_generator.generate(
        args.num_seeds, args.kmer_size, args.max_mismatches, args.max_indels
    )
    print("DONE")
    seeds = sorted(seeds, key=lambda seed: seed.count("0"))
    with open(args.out_file, "w") as fp:
        fp.write(os.linesep.join(seeds))
