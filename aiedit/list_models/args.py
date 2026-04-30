import argparse


def add_subparser(subparsers: argparse._SubParsersAction) -> None:
    subparsers.add_parser("list_models", add_help=False)
