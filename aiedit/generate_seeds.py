import argparse
import itertools
import os
import random


def add_subparser(subparsers: argparse._SubParsersAction) -> None:
    parser: argparse.ArgumentParser = subparsers.add_parser("generate_seeds")
    parser.add_argument(
        "-n", "--num-seeds", help="number of seeds per set", type=int, required=True
    )
    parser.add_argument(
        "-k", "--kmer-sizes", help="kmer sizes", type=int, nargs="+", required=True
    )
    parser.add_argument(
        "-s",
        "--sets-per-k",
        help="number of spaced seed sets per kmer size",
        type=int,
        default=1,
    )
    parser.add_argument("-r", "--random-seed", help="random number generator seed")
    parser.add_argument("-o", "--out-path", help="output directory path", default=".")
    parser.set_defaults(func=main)


def generate(kmer_size: int) -> str:
    weight = int(kmer_size * (1 - random.random() * 0.6))
    weight = min(kmer_size - 2, max(kmer_size - 10, weight))
    half = ["0"] * (kmer_size // 2 - weight // 2) + ["1"] * (weight // 2 - 1)
    random.shuffle(half)
    seed = ["1"] + half + (["1"] if kmer_size % 2 == 1 else []) + half[::-1] + ["1"]
    seed = "".join(seed)
    if len(seed) != kmer_size or seed.count("1") < weight - 1:
        err_msg = f"Tried generating with k={kmer_size} and w={weight}, "
        err_msg += f"result has k={len(seed)} and w={seed.count('1')}"
        raise RuntimeError(err_msg)
    return seed


def save(i_file: int, seeds: list[str], out_dir: str):
    file_name = f"k{len(seeds[0])}_{i_file + 1}.txt"
    with open(os.path.join(out_dir, file_name), "w") as fp:
        fp.write(os.linesep.join(seeds))


def main(args):
    random.seed(args.random_seed)
    print("Generating spaced seeds... ")
    results = [
        ["1" * k] + [generate(k) for _ in range(args.num_seeds - 1)]
        for k in itertools.chain(*itertools.repeat(args.kmer_sizes, args.sets_per_k))
    ]
    print("Done, saving results...")
    for i, seeds in enumerate(results):
        save(i // len(args.kmer_sizes), seeds, args.out_path)
    print(f"Saved {len(results)} files")
