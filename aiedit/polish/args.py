import argparse
import importlib.resources
import os

DEFAULT_MODEL = importlib.resources.files("aiedit.pretrained").joinpath("s3m5i5.pt")


def add_subparser(subparsers: argparse._SubParsersAction) -> None:
    parser: argparse.ArgumentParser = subparsers.add_parser("polish", add_help=False)
    parser.add_argument("--help", action="help", help="show this help message and exit")
    parser.add_argument(
        "-a", "--assembly", help="path to assembly file (required)", required=True
    )
    parser.add_argument(
        "-k",
        "--kmers",
        help="k-mer size or path to kmers bf/cbf file from 'ntstat filter' (default: %(default)s)",
        default="35",
    )
    parser.add_argument(
        "-s",
        "--seeds",
        help="path to spaced seeds bloom filter from 'ntstat filter', "
        "ignored if -k is k-mer size, required otherwise",
    )
    parser.add_argument(
        "-r",
        "--reads",
        help="path to sequencing reads, required if -k is k-mer size or -s is not set",
        nargs="*",
        default=[],
    )
    parser.add_argument(
        "-h",
        "--hist-model",
        help="path to k-mer histogram model from 'ntstat hist'",
    )
    parser.add_argument(
        "-m",
        "--model",
        help="path to pretrained edit pattern model (default: %(default)s)",
        default=str(DEFAULT_MODEL),
    )
    parser.add_argument(
        "-p",
        "--hit-prob",
        help="hit probability threshold (default: %(default).2f)",
        type=float,
        default=0.5,
    )
    parser.add_argument(
        "-y",
        "--num-tries",
        help="number of edit patterns to try (default: %(default)d)",
        type=int,
        default=3,
    )
    parser.add_argument(
        "-g",
        "--max-gap",
        help="maximum gap fill size (default: %(default)d)",
        type=int,
        default=20,
    )
    parser.add_argument(
        "-t",
        "--threads",
        help="number of threads (default: %(default)d)",
        type=int,
        default=os.cpu_count() // 2,
    )
    parser.add_argument(
        "--ntedit",
        help="polish results with ntEdit when done",
        action=argparse.BooleanOptionalAction,
        default=True,
    )
    parser.add_argument(
        "-o",
        "--out-path",
        help="output directory path (default: current directory)",
        default=".",
    )
