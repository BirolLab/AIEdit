import argparse


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
