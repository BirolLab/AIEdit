import argparse
import dataclasses
import itertools
import os
import random
import re
import sys

import matplotlib.pyplot as plt
import prettytable
import torch
import tqdm


class PatternDetector(torch.nn.Module):

    def __init__(self, seeds: list[str], pattern_length: int):
        super().__init__()
        num_channels = 4
        signature_length = pattern_length + len(seeds[0]) - 1
        flattened_size = num_channels * signature_length * len(seeds)
        self._conv1 = torch.nn.Conv2d(1, num_channels, 5, padding="same")
        self._fc1 = torch.nn.Linear(flattened_size, pattern_length)

    def forward(self, signature):
        y = self._conv1(signature)
        y = torch.flatten(y)
        y = self._fc1(y).unsqueeze(-1)
        return y


@dataclasses.dataclass
class Dataset:
    x_train: list[torch.Tensor]
    y_train: list[torch.Tensor]
    x_test: list[torch.Tensor]
    y_test: list[torch.Tensor]


@dataclasses.dataclass
class EpochStats:
    training_loss: float
    validation_error: float


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", help="directory to store results", default=".")
    parser.add_argument("-w",
                        help="maximum pattern length",
                        default=5,
                        type=int)
    parser.add_argument("-e",
                        help="number of training epochs",
                        default=10,
                        type=int)
    parser.add_argument("-a",
                        help="Data augmentation ratio",
                        default=0.1,
                        type=float)
    parser.add_argument(
        "seeds",
        help="spaced seed patterns"
        "(or path to text file with seeds in separate lines)",
        nargs="+",
    )
    args = parser.parse_args()
    if not re.match(r"^[01]+$", args.seeds[0]):
        with open(args.seeds[0]) as f:
            args.seeds = list(map(str.strip, f.readlines()))
    if len(set(map(len, args.seeds))) != 1:
        msg = "Seed patterns should be the same length"
        print(msg, file=sys.stderr)
        exit(1)
    return args


def print_model_summary(model: PatternDetector) -> None:
    table = prettytable.PrettyTable(["Modules", "Parameters"])
    total_params = 0
    for name, parameter in model.named_parameters():
        if not parameter.requires_grad:
            continue
        params = parameter.numel()
        table.add_row([name, params])
        total_params += params
    table.add_row(["TOTAL", total_params])
    print(table)


def get_pattern_strings(pattern_length: int) -> list[str]:
    pattern_strings = []
    for i in range(2**(pattern_length - 1)):
        pattern_strings.append("1" + bin(i)[2:].zfill(pattern_length - 1))
    return pattern_strings


def get_signature(seeds: list[str], pattern_string: str) -> torch.Tensor:
    seed_length = len(seeds[0])
    padding = "0" * (seed_length - 1)
    pattern = padding + pattern_string + padding
    signature_length = len(pattern_string) + len(padding)
    signature = torch.ones(1, signature_length, len(seeds))
    for i, j in itertools.product(range(signature_length), range(len(seeds))):
        has_miss = False
        for pos in range(seed_length):
            if seeds[j][pos] == "1" and pattern[i + pos] == "1":
                has_miss = True
        if has_miss:
            signature[0][i][j] = 0
    return signature


def get_pattern_tensor(pattern_string: str) -> torch.Tensor:
    pattern = list(map(float, pattern_string))
    return torch.FloatTensor(pattern).unsqueeze(-1)


def shuffle_data(signatures: list[torch.Tensor],
                 patterns: list[torch.Tensor]) -> Dataset:
    shuffled = list(zip(signatures, patterns))
    random.shuffle(shuffled)
    x, y = tuple(zip(*shuffled))
    return list(x), list(y)


def augment_data(data: Dataset, ratio: float) -> None:
    x_train, y_train, x_test, y_test = [], [], [], []
    for x, y in zip(data.x_train, data.y_train):
        miss_positions = []
        for i, j in itertools.product(range(x.size()[1]), range(x.size()[2])):
            if x[0][i][j] == 0.0:
                miss_positions.append((i, j))
        random.shuffle(miss_positions)
        miss_positions = miss_positions[:int(len(miss_positions) * ratio * 2)]
        for_train = True
        for i, j in miss_positions:
            x_c, y_c = torch.clone(x), torch.clone(y)
            x_c[0][i][j] = 1.0
            if for_train:
                x_train.append(x_c)
                y_train.append(y_c)
            else:
                x_test.append(x_c)
                y_test.append(y_c)
            for_train = not for_train
    data.x_train.extend(x_train)
    data.y_train.extend(y_train)
    data.x_test.extend(x_test)
    data.y_test.extend(y_test)


def prepare_data(seeds: list[str], pattern_length: int,
                 augmentation_ratio: float) -> Dataset:
    pattern_strings = get_pattern_strings(pattern_length)
    x_train = [get_signature(seeds, p) for p in pattern_strings]
    y_train = [get_pattern_tensor(p) for p in pattern_strings]
    x_train, y_train = shuffle_data(x_train, y_train)
    data = Dataset(x_train, y_train, [], [])
    augment_data(data, augmentation_ratio)
    return data


def get_validation_error(model: PatternDetector, data: Dataset) -> float:
    num_accurate = 0
    with torch.no_grad():
        for x, y_true in zip(data.x_test, data.y_test):
            y_pred = torch.round(torch.sigmoid(model(x)))
            if torch.equal(y_pred, y_true):
                num_accurate += 1
    return 1 - num_accurate / len(data.x_test)


def train(model: PatternDetector, data: Dataset,
          num_epochs: int) -> list[EpochStats]:
    loss_function = torch.nn.BCEWithLogitsLoss()
    optimizer = torch.optim.AdamW(model.parameters(), lr=0.001)
    epochs = tqdm.tqdm(
        range(num_epochs),
        file=sys.stdout,
        unit="epochs",
        leave=False,
        desc="Training",
    )
    stats = []
    for _ in epochs:
        total_loss = 0
        for x, y_true in zip(data.x_train, data.y_train):
            y_pred = model(x)
            loss = loss_function(y_pred, y_true)
            total_loss += loss.item()
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
        epoch_stats = EpochStats(
            training_loss=total_loss / len(data.x_train),
            validation_error=get_validation_error(model, data),
        )
        stats.append(epoch_stats)
        epochs.set_postfix({"loss": f"{epoch_stats.training_loss:.3f}"})
    return stats


def plot_training_stats(stats: list[EpochStats], out_path: str) -> None:
    fig, ax = plt.subplots(1, 2, figsize=(8, 4), dpi=300)
    ax[0].plot([e.training_loss for e in stats])
    ax[0].set_xlabel("Epoch")
    ax[0].set_ylabel("Training loss")
    ax[1].plot([e.validation_error for e in stats])
    ax[1].set_xlabel("Epoch")
    ax[1].set_ylabel("Validation error")
    fig.tight_layout()
    plt.savefig(os.path.join(out_path, "training.png"))


def main():
    args = parse_args()
    model = PatternDetector(args.seeds, args.w)
    print_model_summary(model)
    print("Preparing data... ", end="", flush=True)
    data = prepare_data(args.seeds, args.w, args.a)
    print("\b\b\b\b DONE")
    print(f"  - Training samples: {len(data.x_train)}")
    print(f"  - Testing samples: {len(data.x_test)}")
    training_stats = train(model, data, args.e)
    print("Training DONE")
    print("Saving results... ", end="", flush=True)
    traced_script_module = torch.jit.trace(model, data.x_train[0])
    traced_script_module.save(os.path.join(args.o, "model.pt"))
    plot_training_stats(training_stats, args.o)
    print("\b\b\b\b DONE")


if __name__ == "__main__":
    main()
