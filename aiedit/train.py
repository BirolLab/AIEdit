import argparse
import glob
import math
import os
import random
import signal
import time

import torch.utils.data
import torchinfo

from aiedit import data, utils
from aiedit.model import Model


def add_subparser(subparsers: argparse._SubParsersAction) -> None:
    parser: argparse.ArgumentParser = subparsers.add_parser("train")
    parser.add_argument(
        "-s",
        "--seeds",
        help="path to spaced seed files for training",
        required=True,
        nargs="+",
    )
    parser.add_argument(
        "-v",
        "--val-seeds",
        help="path to spaced seed files for validation",
        nargs="+",
        default=[],
    )
    parser.add_argument(
        "-e", "--max-edits", help="maximum number of edits", type=int, default=10
    )
    parser.add_argument(
        "-d", "--model-dim", help="model dimensionality", type=int, default=32
    )
    parser.add_argument(
        "-n", "--num-epochs", help="number of training epochs", type=int, default=10
    )
    parser.add_argument(
        "-o", "--out-path", help="output model path", default="model.pt"
    )
    parser.set_defaults(func=main)


def train_epoch(model, optimizer, train_data) -> float:
    total_loss = 0
    bce_loss = torch.nn.BCEWithLogitsLoss()
    cat_loss = torch.nn.CrossEntropyLoss()
    for x, y_true in train_data:
        y_pred = model(*x)
        if y_true[1] is not None:
            loss = bce_loss(y_pred[1], y_true[1])
            mis_rate = 1 / 2 ** y_pred[1].size(-1)
        else:
            loss = cat_loss(y_pred[2], y_true[2])
            mis_rate = 1
        loss += bce_loss(y_pred[0], y_true[0]) * mis_rate
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
        total_loss += loss.item()
    return total_loss / len(train_data)


@torch.no_grad
def validate(model, val_data) -> tuple[float, float]:
    loss, acc = 0.0, 0.0
    bce_loss = torch.nn.BCEWithLogitsLoss()
    cat_loss = torch.nn.CrossEntropyLoss()
    for x, y_true in val_data:
        y_pred = model(*x)
        if y_true[1] is not None:
            loss += bce_loss(y_pred[1], y_true[1])
            mis_rate = 1 / 2 ** y_pred[1].size(-1)
        else:
            loss += cat_loss(y_pred[2], y_true[2])
            mis_rate = 1
        loss += bce_loss(y_pred[0], y_true[0]) * mis_rate
        if y_true[1] is not None and y_pred[0].item() <= 0:
            acc += int(((y_pred[1] >= 0) == y_true[1]).all())
        elif y_true[2] is not None and y_pred[0].item() > 0:
            acc += int(y_true[2].argmax() == y_pred[2].argmax())
    return loss / len(val_data), acc / len(val_data)


def main(args):
    signal.signal(signal.SIGINT, signal.SIG_DFL)

    print("Loading spaced seed files...")
    train_seed_paths = [p for w in args.seeds for p in glob.glob(w, recursive=True)]
    val_seed_paths = [p for w in args.val_seeds for p in glob.glob(w, recursive=True)]
    seeds = [utils.load_seeds(path) for path in train_seed_paths]
    val_seeds = [utils.load_seeds(path) for path in val_seed_paths]
    seed_lengths = set(map(len, seeds)) | set(map(len, val_seeds))
    assert len(seed_lengths) == 1, "All files must have the same number of seeds"
    num_seeds = next(iter(seed_lengths))
    print(f"Loaded {len(seeds)} spaced seed sets for training")
    print(f"Loaded {len(val_seeds)} spaced seed sets for validation")
    print(f"Number of spaced seeds per set: {num_seeds}")

    if os.path.exists(args.out_path):
        model, optimizer = utils.load_checkpoint(args.out_path)
        print("Training state loaded from checkpoint")
    else:
        model = Model(len(seeds[0]), args.max_edits, args.model_dim)
        optimizer = torch.optim.AdamW(model.parameters())
        input_shape = [(30, num_seeds), (32, num_seeds * (args.max_edits + 1) + 1)]
        torchinfo.summary(model, input_shape, col_width=15)

    print("Generating data...")
    train_data = [x for s in seeds for x in data.generate_dataset(s, args.max_edits)]
    val_data = [x for s in val_seeds for x in data.generate_dataset(s, args.max_edits)]
    print(f"Number of training samples: {len(train_data)}")
    print(f"Number of validation samples: {len(val_data)}")

    print("Training...")
    checkpoint = {
        "num_seeds": torch.tensor(num_seeds, dtype=torch.int),
        "max_edits": torch.tensor(args.max_edits, dtype=torch.int),
        "model_dim": torch.tensor(args.model_dim, dtype=torch.int),
    }
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
        checkpoint["model"] = model.state_dict()
        checkpoint["optimizer"] = optimizer.state_dict()
        torch.save(checkpoint, args.out_path)
        end_time = time.perf_counter()
        epoch_log["time"] = f"{end_time - start_time:.3f}s"
        i_epoch_str = str(i_epoch + 1).rjust(int(math.log(args.num_epochs)))
        log_str = ", ".join(f"{k}={v}" for k, v in epoch_log.items())
        print(f"Epoch {i_epoch_str}:", log_str)
