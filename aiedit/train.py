import argparse
import collections
import datetime
import os
import signal
import sys
import time

import btllib
import torch
import tqdm

from aiedit import core
from aiedit.edit_model import EditModel
from aiedit.model_trainer import ModelTrainer


class ProgressManager:

    SLIDING_WINDOW_SIZE = 10

    def __init__(self, seq_len: int):
        self._seq_len = seq_len
        self._pbar = tqdm.tqdm(
            file=sys.stdout,
            unit="bps",
            total=seq_len,
            leave=False,
            unit_scale=True,
            mininterval=1,
        )
        self._losses = collections.deque([0] * ProgressManager.SLIDING_WINDOW_SIZE)
        self._rewards = collections.deque([0] * ProgressManager.SLIDING_WINDOW_SIZE)
        self._loss_sum, self._reward_sum = 0, 0

    def update(self, region: tuple[int], loss: float, reward: float, num_steps: int):
        self._losses.append(loss)
        self._rewards.append(reward)
        self._loss_sum += loss - self._losses.popleft()
        self._reward_sum += reward - self._rewards.popleft()
        avg_loss = self._loss_sum / ProgressManager.SLIDING_WINDOW_SIZE
        avg_reward = self._reward_sum / ProgressManager.SLIDING_WINDOW_SIZE
        postfix = f"loss={avg_loss:.4f}, reward={avg_reward:.2f}, steps={num_steps}"
        self._pbar.update(region[1] - self._pbar.n)
        self._pbar.set_postfix_str(postfix, refresh=False)

    def stop(self) -> None:
        self._pbar.update(self._seq_len - self._pbar.n)
        self._pbar.close()
        timestamp = datetime.datetime.now().isoformat()
        postfix = self._pbar.postfix
        if postfix:
            tqdm.tqdm.write(f"[{timestamp}] {postfix}")


class CheckpointSaver:

    def __init__(self, model: EditModel, optimizer: torch.optim.Optimizer, path: str):
        self._model = model
        self._optimizer = optimizer
        self._path = path

    def save(self, loss_history: list[float], reward_history: list[float]):
        checkpoint = {
            "model": self._model.state_dict(),
            "optimizer": self._optimizer.state_dict(),
            "num_seeds": torch.tensor(self._model._num_seeds, dtype=torch.int),
            "model_dim": torch.tensor(self._model._model_dim, dtype=torch.int),
        }
        torch.save(checkpoint, self._path)
        if len(loss_history) == 0:
            return
        history_file = self._path.replace(".pt", "") + "_history.csv"
        rows_iter = zip(loss_history, reward_history)
        with open(history_file, "a") as fp:
            fp.write(os.linesep.join(f"{loss},{reward}" for loss, reward in rows_iter))
            fp.write(os.linesep)
        loss_history.clear()
        reward_history.clear()
        tqdm.tqdm.write(f"[{datetime.datetime.now().isoformat()}] Checkpoint saved")


def load_checkpoint(path: str, num_kmer_model_seeds: int, exploration_factor: float):
    checkpoint = torch.load(path, weights_only=True)
    num_seeds = checkpoint["num_seeds"].item()
    assert num_seeds == num_kmer_model_seeds
    model_dim = checkpoint["model_dim"].item()
    model = EditModel(num_seeds, model_dim, exploration_factor)
    model.load_state_dict(checkpoint["model"])
    optim = torch.optim.AdamW(model.parameters(), 0.01)
    optim.load_state_dict(checkpoint["optimizer"])
    return model, optim


def add_subparser(subparsers: argparse._SubParsersAction) -> None:
    parser: argparse.ArgumentParser = subparsers.add_parser("train")
    parser.add_argument("input_file", help="path to input assembly")
    parser.add_argument("-c", help="path to counting bloom filter", required=True)
    parser.add_argument("-k", help="path to k-mer histogram model", required=True)
    parser.add_argument("-s", help="path to spaced seeds file", required=True)
    parser.add_argument("-p", help="hit probability threshold", type=float, default=0.5)
    parser.add_argument("-y", help="maximum number of edits", type=int, default=5)
    parser.add_argument("-d", help="model dimensionality", type=int, default=32)
    parser.add_argument("-e", help="model exploration factor", type=float, default=0.1)
    parser.add_argument("-f", help="checkpoint save frequency", type=int, default=1000)
    parser.add_argument("-o", help="checkpoint path", default="checkpoint.pt")
    parser.set_defaults(func=main)


def main(args):
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    print("Loading k-mer model... ", end="", flush=True)
    start_time = time.perf_counter()
    kmer_model = core.KmerModel(args.c, args.k, args.s)
    end_time = time.perf_counter()
    print(f"DONE ({end_time - start_time:.1f}s)")
    if os.path.exists(args.o):
        print("Loading model from checkpoint")
        model, optim = load_checkpoint(args.o, len(kmer_model.seeds), args.e)
    else:
        model = EditModel(len(kmer_model.seeds), args.d, args.e)
        optim = torch.optim.AdamW(model.parameters(), 0.01)
    model.train()
    model.summary()
    trainer = ModelTrainer(model, optim, kmer_model, args.p, args.y, args.f)
    seq_reader = btllib.SeqReader(args.input_file, btllib.SeqReaderFlag.LONG_MODE)
    checkpoint_saver = CheckpointSaver(model, optim, args.o)
    for record in seq_reader:
        progress_manager = ProgressManager(len(record.seq))
        tqdm.tqdm.write(f"Training on {record.id}...")
        trainer.train(record.seq, progress_manager.update, checkpoint_saver.save)
        progress_manager.stop()
