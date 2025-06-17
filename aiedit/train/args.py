import argparse


def add_subparser(subparsers: argparse._SubParsersAction) -> None:
    parser: argparse.ArgumentParser = subparsers.add_parser("train")
    parser.add_argument(
        "-s", "--seeds", help="training spaced seed files", required=True, nargs="+"
    )
    parser.add_argument(
        "-v", "--val-seeds", help="validation spaced seed files", nargs="+", default=[]
    )
    parser.add_argument(
        "-m", "--max-mismatches", help="mismatch window size", type=int, default=5
    )
    parser.add_argument(
        "-i", "--max-indels", help="indel window size", type=int, default=5
    )
    parser.add_argument(
        "-d", "--model-dim", help="model dimensionality", type=int, default=32
    )
    parser.add_argument(
        "-e", "--num-epochs", help="number of training epochs", type=int, default=50
    )
    parser.add_argument(
        "-o", "--out-path", help="output model path", default="model.pt"
    )
