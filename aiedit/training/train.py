import argparse
import glob
import math
import os
import pathlib
import random
import time

import torch
import torch.nn.functional as F
import tqdm

from aiedit.model import Model
from aiedit.training import data


def add_subparser(subparsers: argparse._SubParsersAction) -> None:
    parser: argparse.ArgumentParser = subparsers.add_parser("train")
    parser.add_argument(
        "-s", "--seeds", help="training spaced seed files", required=True, nargs="+"
    )
    parser.add_argument(
        "-v", "--val-seeds", help="validation spaced seed files", nargs="+", default=[]
    )
    parser.add_argument(
        "-m", "--max-mismatches", help="mismatch window size", type=int, default=5
    )
    parser.add_argument(
        "-i", "--max-indels", help="indel window size", type=int, default=10
    )
    parser.add_argument(
        "-d", "--model-dim", help="model dimensionality", type=int, default=8
    )
    parser.add_argument(
        "-e", "--num-epochs", help="number of training epochs", type=int, default=10
    )
    parser.add_argument(
        "-o", "--out-path", help="output model path", default="model.pt"
    )
    parser.set_defaults(func=main)


def load_seed_sets(pattern: str) -> list[list[str]]:
    seed_paths = [p for w in pattern for p in glob.glob(w, recursive=True)]
    seeds = []
    for path in seed_paths:
        with open(path) as fp:
            seeds.append(list(map(str.strip, fp.readlines())))
    return seeds


def calculate_loss(
    y_pred: torch.FloatTensor, y_true: torch.FloatTensor
) -> torch.FloatTensor:
    loss = F.binary_cross_entropy_with_logits(y_pred[0], y_true[0])
    is_mismatch = y_true[1] is not None
    if is_mismatch:
        loss += F.binary_cross_entropy_with_logits(y_pred[1], y_true[1])
    else:
        loss += F.cross_entropy(y_pred[2], y_true[2])
    return loss


def train_epoch(model, optimizer, train_data) -> float:
    total_loss = 0
    for x, y_true in train_data:
        y_pred = model(*x)
        loss = calculate_loss(y_pred, y_true)
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
        total_loss += loss.item()
    return total_loss / len(train_data)


def check_prediction(y_pred: torch.FloatTensor, y_true: torch.FloatTensor) -> bool:
    if y_true[1] is not None and y_pred[0].item() <= 0:
        return ((y_pred[1] >= 0) == y_true[1]).sum(), y_pred[1].size(1)
    elif y_true[2] is not None and y_pred[0].item() > 0:
        return y_true[2].argmax() == y_pred[2].argmax(), 1
    return 0, 1


@torch.no_grad()
def validate(model, val_data) -> tuple[float, float]:
    loss, num_true, num_pred = 0.0, 0, 0
    for x, y_true in val_data:
        y_pred = model(*x)
        loss += calculate_loss(y_pred, y_true)
        nt, np = check_prediction(y_pred, y_true)
        num_true += nt
        num_pred += np
    return loss / len(val_data), num_true / num_pred


def create_dataset(name, seeds, max_mismatches, max_indels):
    dataset = []
    num_samples = (2 ** (max_mismatches - 1) + 2 * max_indels) * len(seeds)
    pbar = tqdm.tqdm(desc=name, total=num_samples)
    for seed in seeds:
        for x in data.generate_dataset(seed, max_mismatches, max_indels):
            dataset.append(x)
            pbar.update()
    return dataset


def main(args):
    print("Loading spaced seed files...")
    seeds = load_seed_sets(args.seeds)
    val_seeds = load_seed_sets(args.val_seeds)
    seed_lengths = set(map(len, seeds)) | set(map(len, val_seeds))
    assert len(seed_lengths) == 1, "All files must have the same number of seeds"
    num_seeds = next(iter(seed_lengths))
    print(f"Loaded {len(seeds)} spaced seed sets for training")
    print(f"Loaded {len(val_seeds)} spaced seed sets for validation")
    print(f"Number of spaced seeds per set: {num_seeds}")

    out_path = pathlib.Path(args.out_path)
    checkpoint_path = out_path.parent.joinpath(out_path.stem).joinpath("-checkpoint.pt")

    max_edits = args.max_mismatches, args.max_indels
    if os.path.exists(checkpoint_path):
        model, optimizer_state = Model.from_checkpoint(checkpoint_path)
        optimizer = torch.optim.AdamW(model.parameters())
        optimizer.load_state_dict(optimizer_state)
        print("Training state loaded from checkpoint")
    else:
        model = Model(len(seeds[0]), *max_edits, args.model_dim)
        optimizer = torch.optim.AdamW(model.parameters())
        model.print_summary()

    print("Generating data...")
    train_data = create_dataset("Training data", seeds, *max_edits)
    val_data = create_dataset("Validation data", val_seeds, *max_edits)

    print("Training...")
    for i_epoch in range(args.num_epochs):
        start_time = time.perf_counter()
        epoch_log = dict()
        random.shuffle(train_data)
        loss = train_epoch(model, optimizer, train_data)
        epoch_log["loss"] = f"{loss:.3f}"
        if len(val_data) > 0:
            val_loss, acc = validate(model, val_data)
            epoch_log["val_loss"] = f"{val_loss:.3f}"
            epoch_log["acc"] = f"{acc:.4f}"
        model.save(checkpoint_path, optimizer.state_dict())
        end_time = time.perf_counter()
        epoch_log["time"] = f"{end_time - start_time:.3f}s"
        i_epoch_str = str(i_epoch + 1).rjust(int(math.log(args.num_epochs)))
        log_str = ", ".join(f"{k}={v}" for k, v in epoch_log.items())
        print(f"Epoch {i_epoch_str}:", log_str)

    model_ts = torch.jit.script(model)
    model_ts.save(args.out_path)
