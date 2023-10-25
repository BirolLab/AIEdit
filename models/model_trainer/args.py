import argparse
import re
import sys


def generate_default_seeds(kmer_length: int, pattern_length: int) -> list[str]:
    seeds = []
    for i in range(1, pattern_length + 1):
        gap = "0" * i if i % 2 == 1 else "0" * i + "1" + "0" * i
        cares = "1" * ((kmer_length - len(gap)) // 2)
        seeds.append(cares + gap + cares)
    return seeds


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser("AIEdit pattern detector training script")
    parser.add_argument(
        "-s",
        help="spaced seed patterns separated by commas "
        "(e.g., 1110111,1101011), or k-mer length (e.g., 64)",
        required=True,
    )
    parser.add_argument("-w", help="maximum pattern length", default=5, type=int)
    parser.add_argument("-e", help="number of training epochs", default=20, type=int)
    parser.add_argument("-n", help="number of samples per class", default=20, type=int)
    parser.add_argument(
        "-fpr", help="false positive rate for simulation", default=0.001, type=float
    )
    parser.add_argument("-o", help="output model file path", default="model.json")
    parser.add_argument(
        "--plot-stats",
        action="store_true",
        help="plot training stats (stored in training.png next to the model)",
    )
    args = parser.parse_args()
    if not re.match(r"^[01,]+$", args.s):
        args.s = generate_default_seeds(int(args.s), args.w)
    else:
        args.s = args.s.split(",")
    if len(set(map(len, args.s))) != 1:
        print("Error: Seed patterns (-s) should be the same length", file=sys.stderr)
        parser.print_help()
        exit(1)
    return args
