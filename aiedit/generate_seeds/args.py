import argparse


def add_subparser(subparsers: argparse._SubParsersAction) -> None:
    parser: argparse.ArgumentParser = subparsers.add_parser("generate_seeds")
    parser.add_argument(
        "-n", "--num-seeds", help="number of seeds (required)", type=int, required=True
    )
    parser.add_argument(
        "-k", "--kmer-size", help="kmer size (required)", type=int, required=True
    )
    parser.add_argument(
        "-m",
        "--max-mismatches",
        help="mismatch window size (default: %(default)d)",
        type=int,
        default=5,
    )
    parser.add_argument(
        "-i",
        "--max-indels",
        help="indel window size (default: %(default)d)",
        type=int,
        default=5,
    )
    parser.add_argument(
        "-r", "--random-seed", help="random number generator seed", type=int
    )
    parser.add_argument(
        "-p",
        "--pop-size",
        help="genetic algorithm's population size",
        type=int,
        default=10,
    )
    parser.add_argument(
        "-g",
        "--num-gens",
        help="genetic algorithm's number of generations",
        type=int,
        default=20,
    )
    parser.add_argument(
        "-u",
        "--mutation",
        help="genetic algorithm's mutation probability",
        type=float,
        default=0.25,
    )
    parser.add_argument(
        "-o",
        "--out-file",
        help="path to store spaced seeds file",
    )
