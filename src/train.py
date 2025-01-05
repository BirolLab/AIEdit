import argparse
import json
import os
import random

import aiedit_torch_extensions as ext  # type: ignore
import torch
import torchinfo
import tqdm


class Model(torch.nn.Module):

    def __init__(self, num_seeds: int, max_indels: int, max_k: int, dim: int):
        super(Model, self).__init__()
        self.num_seeds = torch.jit.Attribute(num_seeds, int)
        self.max_indels = torch.jit.Attribute(max_indels, int)
        self.max_k = torch.jit.Attribute(max_k, int)
        probs_dim = num_seeds + 2 * max_indels + 1
        self.input_sizes = [(100, probs_dim), (num_seeds, max_k), (max_indels, 5)]
        self.register_buffer("pos_enc", ext.positional_encoding(2000, dim))
        self.probs_proj = torch.nn.Linear(probs_dim, dim)
        self.seeds_proj = torch.nn.Linear(max_k, dim)
        self.edits_proj = torch.nn.Linear(5, dim)
        self.seeds2probs = torch.nn.Transformer(dim, 4, 1, 1, dim, batch_first=True)
        self.probs2edits = torch.nn.Transformer(dim, 4, 1, 1, dim, batch_first=True)
        self.out = torch.nn.Linear(dim, 5)

    @torch.jit.ignore
    def summary(self):
        torchinfo.summary(self, input_size=self.input_sizes)

    def forward(self, x_probs, x_seeds, x_edits):
        x_probs = self.probs_proj(x_probs) + self.pos_enc[: x_probs.size(0), :]
        x_seeds = self.seeds_proj(x_seeds)
        x_edits = self.edits_proj(x_edits) + self.pos_enc[: x_edits.size(0), :]
        x_probs = self.seeds2probs(x_seeds, x_probs)
        mask = torch.ones(x_edits.size(0), x_edits.size(0))
        mask = torch.triu(mask * float("-inf"), diagonal=1)
        y = self.probs2edits(x_probs, x_edits, tgt_mask=mask, tgt_is_causal=True)
        return self.out(y)


def parse_args():
    default_t = torch.get_num_threads()
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", help="path to dataset", required=True, nargs="+")
    parser.add_argument("-b", help="batch size", type=int, default=32)
    parser.add_argument("-n", help="number of epochs", type=int, default=1)
    parser.add_argument("-t", help="number of threads", type=int, default=default_t)
    parser.add_argument("-k", help="maximum kmer length", type=int, default=128)
    parser.add_argument("-m", help="hidden dimension", type=int, default=64)
    parser.add_argument("-o", help="output files prefix", default="model")
    return parser.parse_args()


def load_data(paths: list[str]):
    data = [torch.load(file, weights_only=True) for file in paths]
    max_indels = set(d["max_indels"] for d in data)
    num_seeds = set(len(d["seeds"]) for d in data)
    assert len(max_indels) == 1, "All datasets must have the same maximum indel length"
    assert len(num_seeds) == 1, "All datasets must have the same number of seeds"
    return data, max_indels.pop(), num_seeds.pop()


def weighted_ce_loss(logits, targets, reduction_factor=0.1):
    ce_loss = torch.nn.functional.cross_entropy(logits, targets, reduction="none")
    ce_loss[(logits.argmax(dim=1) == targets) & (targets == 0)] *= reduction_factor
    return ce_loss.sum()


def train(model, optimizer, data, batch_size, i_epoch):
    x_seeds = [ext.encode_seeds(d["seeds"], model.max_k.value) for d in data]
    indices = [(i, j) for i, d in enumerate(data) for j in range(len(d["data"]))]
    random.shuffle(indices)
    num_true, num_out = 0, 0
    i_pattern = 1
    loss, epoch_loss = 0, 0
    loss_history, acc_history = [], []
    pbar = tqdm.tqdm(indices, ascii=" >=", desc=f"Epoch {i_epoch + 1}", ncols=100)
    for i_data, i_sample in pbar:
        x, y = data[i_data]["data"][i_sample]
        y_pred = model(x, x_seeds[i_data], y[:-1, :])
        y_true = y[1:, :].argmax(dim=1)
        loss += weighted_ce_loss(y_pred, y_true)
        if i_pattern % batch_size == 0:
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
            epoch_loss += loss.item()
            loss = 0
        num_ignore = ((y_true == 0) & (y_pred.argmax(dim=1) == y_true)).sum()
        num_true += (y_pred.argmax(dim=1) == y_true).sum() - num_ignore
        num_out += y_true.size(0) - num_ignore
        acc = (num_true / num_out).item()
        acc_history.append(acc)
        avg_loss = epoch_loss / i_pattern
        loss_history.append(avg_loss)
        pbar.set_postfix_str(f"loss={avg_loss:.4f}, acc={acc:.4f}")
        i_pattern += 1
    return loss_history, acc_history


def main():
    args = parse_args()
    print("Loading datasets...")
    datasets, max_indels, num_seeds = load_data(args.d)
    model = Model(num_seeds, max_indels, args.k, args.m)
    optimizer = torch.optim.AdamW(model.parameters())
    if os.path.isfile(args.o):
        checkpoint = torch.load(args.o, weights_only=True)
        model.load_state_dict(checkpoint["model_state_dict"])
        optimizer.load_state_dict(checkpoint["optimizer_state_dict"])
        print("Model state loaded from checkpoint")
    model.summary()
    torch.set_num_threads(args.t)
    print(f"Maximum indel length: {max_indels}")
    print(f"Number of seeds: {num_seeds}")
    print(f"Number of samples: {sum(len(d['data']) for d in datasets)}")
    print(f"Using {torch.get_num_threads()} threads")
    history = {"loss": [], "acc": []}
    for i_epoch in range(args.n):
        loss, acc = train(model, optimizer, datasets, args.b, i_epoch)
        history["loss"].extend(loss)
        history["acc"].extend(acc)
        state_dict = dict()
        state_dict["model_state_dict"] = model.state_dict()
        state_dict["optimizer_state_dict"] = optimizer.state_dict()
        torch.save(state_dict, args.o + "_checkpoint.pt")
        history_file = open(args.o + "_history.json", "w")
        json.dump(history, history_file)
        history_file.close()
    torch.jit.script(model).save(args.o + "_jit.pt")


if __name__ == "__main__":
    main()
