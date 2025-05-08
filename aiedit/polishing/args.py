import argparse
import importlib.resources
import os

import aiedit.polishing
import aiedit.polishing.polish

DEFAULT_MODEL_PATH = importlib.resources.files("aiedit.models").joinpath("s9m5i5.pt")


def add_subparser(subparsers: argparse._SubParsersAction) -> None:
    parser: argparse.ArgumentParser = subparsers.add_parser("polish", add_help=False)
    parser.add_argument("--help", action="help", help="show this help message and exit")
    parser.add_argument("input_file", help="path to assembly file")
    parser.add_argument(
        "-r",
        "--reads",
        help="path to sequencing reads, required if -k is a number or -s is not set",
        nargs="*",
        default=[],
    )
    parser.add_argument(
        "-k",
        "--kmers",
        help="k-mer size or path to kmers bf/cbf file from 'ntstat filter' (required)",
        required=True,
    )
    parser.add_argument(
        "-s",
        "--seeds",
        help="path to spaced seeds bloom filter from 'ntstat filter', "
        "ignored if -k is k-mer size, required otherwise",
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
        default=str(DEFAULT_MODEL_PATH),
    )
    parser.add_argument(
        "-p",
        "--hit-prob",
        help="hit probability threshold (default: %(default).2f)",
        type=float,
        default=0.5,
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
    parser.set_defaults(func=aiedit.polishing.polish.main)
