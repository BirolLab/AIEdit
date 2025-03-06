import torch

from aiedit import core


def buffer2d_to_tensor(buffer: core.Buffer2D) -> torch.Tensor:
    shape = (buffer.num_rows, buffer.num_cols)
    return torch.frombuffer(buffer, dtype=torch.float).view(*shape)
