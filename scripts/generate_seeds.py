import argparse
import os
import random


def generate_seeds(
    kmer_length: int, pattern_length: int, seed_weight: int
) -> list[str]:
    seeds = ["1" * kmer_length]
    for n_gap in range(1, pattern_length + 1):
        end = "1" + "0" * n_gap + "1"
        w = kmer_length - n_gap * 2
        mid = ["1"] * (w - 4)
        if w > seed_weight:
            for i in random.sample(range(len(mid)), (w - seed_weight) // 2):
                mid[i] = mid[len(mid) - 1 - i] = "0"
        seeds.append(end + "".join(mid) + end)
    return seeds


def _main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-k", help="k-mer length", type=int, required=True)
    parser.add_argument("-w", help="pattern length", type=int, required=True)
    parser.add_argument("-c", help="spaced seed weight", type=int, required=True)
    parser.add_argument("-s", help="random number generator seed", type=int)
    args = parser.parse_args()
    random.seed(args.s)
    print(*generate_seeds(args.k, args.w, args.c), sep=os.linesep)


if __name__ == "__main__":
    _main()
