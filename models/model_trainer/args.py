import argparse
import sys


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser("AIEdit pattern detector training script")
    parser.add_argument(
        "-s",
        help="path to text file containing one spaced seed pattern per line",
        required=True,
    )
    parser.add_argument("-w", help="maximum pattern length", default=5, type=int)
    parser.add_argument("-e", help="number of training epochs", default=20, type=int)
    parser.add_argument("-n", help="number of samples per class", default=20, type=int)
    parser.add_argument(
        "-fpr", help="false positive rate for simulation", default=0.001, type=float
    )
    parser.add_argument("-o", help="output model file path", required=False)
    parser.add_argument(
        "--plot-stats",
        action="store_true",
        help="plot training stats (stored in training.png next to the model)",
    )
    args = parser.parse_args()
    with open(args.s) as fp:
        seeds = list(map(str.strip, fp.readlines()))
    if "1" * len(seeds[0]) in seeds:
        seeds.remove("1" * len(seeds[0]))
    args.s = seeds
    if len(set(map(len, args.s))) != 1:
        print("Error: Seed patterns (-s) should be the same length", file=sys.stderr)
        parser.print_help()
        exit(1)
    return args
