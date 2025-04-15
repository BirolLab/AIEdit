import glob

import torch

from aiedit import core


def load_seeds(path: str) -> list[str]:
    with open(path) as fp:
        seeds = list(map(str.strip, fp.readlines()))
    return seeds


def glob_seed_paths(pattern: str) -> list[list[str]]:
    seed_paths = [p for w in pattern for p in glob.glob(w, recursive=True)]
    return [load_seeds(path) for path in seed_paths]


def encode_seeds(seeds: list[str]) -> torch.FloatTensor:
    x_seeds = torch.tensor([list(map(int, seed)) for seed in seeds])
    return x_seeds.permute(1, 0).float()


def buffer2d_to_tensor(buffer: core.Buffer2D) -> torch.Tensor:
    shape = (buffer.num_rows, buffer.num_cols)
    return torch.frombuffer(buffer, dtype=torch.float).view(*shape)
