import torch

from aiedit import core
from aiedit.model import Model


def load_seeds(path: str) -> list[str]:
    with open(path) as fp:
        seeds = list(map(str.strip, fp.readlines()))
    return seeds


def load_checkpoint(path: str) -> tuple[Model, torch.optim.Optimizer]:
    checkpoint = torch.load(path, weights_only=True)
    args = (checkpoint[k].item() for k in ["num_seeds", "max_edits", "model_dim"])
    model = Model(*args)
    model.load_state_dict(checkpoint["model"])
    optimizer = torch.optim.AdamW(model.parameters())
    optimizer.load_state_dict(checkpoint["optimizer"])
    return model, optimizer


def encode_seeds(seeds: list[str]) -> torch.FloatTensor:
    x_seeds = torch.tensor([list(map(int, seed)) for seed in seeds])
    return x_seeds.permute(1, 0).float()


def buffer2d_to_tensor(buffer: core.Buffer2D) -> torch.Tensor:
    shape = (buffer.num_rows, buffer.num_cols)
    return torch.frombuffer(buffer, dtype=torch.float).view(*shape)
