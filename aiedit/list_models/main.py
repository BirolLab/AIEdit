import argparse
import importlib.resources

import torch

from aiedit.polish.args import DEFAULT_MODEL


def add_subparser(subparsers: argparse._SubParsersAction) -> None:
    subparsers.add_parser("list_models", add_help=False)


def main(_):
    print("Pretrained models:")
    print()
    for path in importlib.resources.files("aiedit.pretrained").iterdir():
        print(path, "(default)" if path == DEFAULT_MODEL else "")
        model = torch.jit.load(path)
        print("Number of seed patterns      ", model.num_seeds)
        print("Maximum mismatches in region ", model.max_mismatches)
        print("Maximum indel length         ", model.max_indels)
        print("Hidden dimension size        ", model.model_dim)
        print()
