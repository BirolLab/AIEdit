import glob
import math
import os
import pathlib
import random
import time

import torch
import tqdm

from aiedit.train import data
from aiedit.train.model import Model


def load_seed_sets(pattern: str) -> list[list[str]]:
    seed_paths = [p for w in pattern for p in glob.glob(w, recursive=True)]
    seeds = []
    for path in seed_paths:
        with open(path) as fp:
            seeds.append(list(map(str.strip, fp.readlines())))
    return seeds


def train_epoch(model, optimizer, train_data) -> float:
    total_loss = 0
    for x_seed, x_sig, y_true in tqdm.tqdm(train_data, disable=None, leave=False):
        y_pred = model(x_seed, x_sig)
        loss = torch.nn.functional.cross_entropy(y_pred, y_true)
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
        total_loss += loss.item()
    return total_loss / len(train_data)


@torch.no_grad()
def validate(model, val_data) -> tuple[float, float]:
    loss, num_true, num_pred = 0.0, 0, 0
    for x_seed, x_sig, y_true in val_data:
        y_pred = model(x_seed, x_sig)
        loss += torch.nn.functional.cross_entropy(y_pred, y_true)
        num_true += int(y_pred.argmax() == y_true.argmax())
        num_pred += 1
    return loss / len(val_data), num_true / num_pred


def create_dataset(name, seeds, max_mismatches, max_indels):
    dataset = []
    num_samples = (2 ** (max_mismatches - 1) + 2 * max_indels) * len(seeds)
    pbar = tqdm.tqdm(desc=name, total=num_samples, disable=None)
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
    checkpoint_path = out_path.parent.joinpath(out_path.stem + "-checkpoint.pt")

    if os.path.exists(checkpoint_path):
        model, optimizer_state = Model.from_checkpoint(checkpoint_path)
        optimizer = torch.optim.AdamW(model.parameters())
        optimizer.load_state_dict(optimizer_state)
        max_edits = model._max_mismatches, model._max_indels
        print("Training state loaded from checkpoint")
    else:
        max_edits = args.max_mismatches, args.max_indels
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
    print(f"Final model saved to {args.out_path}")
