import argparse
import os
import random


def generate_seeds(
    kmer_length: int, pattern_length: int, seed_weight: int
) -> list[str]:
    seeds = ["1" * kmer_length]
    for n_gap in range(1, pattern_length + 1):
        n_gap_cares = random.randrange(1, pattern_length, 2)
        mid = ["0"] * n_gap + ["1"] * n_gap_cares + ["0"] * n_gap
        cares = ["1"] * ((kmer_length - len(mid)) // 2)
        seed = cares + mid + cares
        w = seed.count("1")
        c = "0" if seed_weight < w else "1"
        for i in random.sample(range(2, len(cares) - 1), abs(seed_weight - w) // 2):
            seed[i] = seed[len(seed) - 1 - i] = c
        seeds.append("".join(seed))
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
