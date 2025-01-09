import argparse
import json
import os
import random

import aiedit_torch_extensions as ext  # type: ignore
import torch
import torchinfo
import tqdm


class SeedsEncoder(torch.nn.Module):

    def __init__(self, num_seeds: int, hiden_size: int):
        super(SeedsEncoder, self).__init__()
        self._gru = torch.nn.GRU(num_seeds, hiden_size, batch_first=True)

    def forward(self, x_seeds):
        return self._gru(x_seeds)[1]


class ProbsEncoder(torch.nn.Module):

    def __init__(self, probs_dim: int, hidden_size: int):
        super(ProbsEncoder, self).__init__()
        self._gru = torch.nn.GRU(probs_dim, hidden_size, batch_first=True)

    def forward(self, x_probs, h_seeds):
        return self._gru(x_probs, h_seeds)[1]


class Decoder(torch.nn.Module):

    def __init__(self, hidden_size: int):
        super(Decoder, self).__init__()
        self._gru = torch.nn.GRU(5, hidden_size, batch_first=True)
        self._out = torch.nn.Linear(hidden_size, 5)

    def forward(self, x_edits, h_probs):
        return self._out(self._gru(x_edits, h_probs)[0])


class Model(torch.nn.Module):

    def __init__(self, num_seeds: int, max_indels: int, hidden_size: int):
        super(Model, self).__init__()
        probs_dim = num_seeds + 2 * max_indels + 1
        self._input_dims = [(35, num_seeds), (40, probs_dim), (3, 5)]
        self.num_seeds = torch.jit.Attribute(num_seeds, int)
        self.max_indels = torch.jit.Attribute(max_indels, int)
        self.seeds_encoder = SeedsEncoder(num_seeds, hidden_size)
        self.probs_encoder = ProbsEncoder(probs_dim, hidden_size)
        self.decoder = Decoder(hidden_size)

    @torch.jit.ignore
    def summary(self):
        torchinfo.summary(self, self._input_dims)

    def forward(self, x_seeds, x_probs, x_edits):
        h_seeds = self.seeds_encoder(x_seeds)
        h_probs = self.probs_encoder(x_probs, h_seeds)
        return self.decoder(x_edits, h_probs)


def parse_args():
    default_t = torch.get_num_threads()
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("-d", help="path to dataset(s)", required=True, nargs="+")
    parser.add_argument("-v", help="path to validation data")
    parser.add_argument("-b", help="batch size", type=int, default=1)
    parser.add_argument("-e", help="number of epochs", type=int, default=1)
    parser.add_argument("-t", help="number of threads", type=int, default=default_t)
    parser.add_argument("-h", help="hidden dimension", type=int, default=64)
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
    return ce_loss


def update_accuracy(y_pred, y_true, num_true, num_out):
    num_ignore = ((y_true == 0) & (y_pred.argmax(dim=1) == y_true)).sum()
    num_true += (y_pred.argmax(dim=1) == y_true).sum() - num_ignore
    num_out += y_true.size(0) - num_ignore
    return num_true, num_out


def train(model, optimizer, data, batch_size, i_epoch, val_data):
    x_seeds = [ext.encode_seeds(d["seeds"]) for d in data]
    indices = [(i, j) for i, d in enumerate(data) for j in range(len(d["data"]))]
    random.shuffle(indices)
    num_true, num_out = 0, 0
    i_pattern = 1
    loss = []
    loss_history, acc_history = [], []
    pbar = tqdm.tqdm(indices, desc=f"Epoch {i_epoch + 1}", ncols=0)
    for i_data, i_sample in pbar:
        x_probs, y = data[i_data]["data"][i_sample]
        kmer_size = len(data[i_data]["seeds"][0])
        if x_probs.size(0) > 3 * kmer_size or y.size(0) > data[i_data]["max_indels"]:
            continue
        y_pred = model(x_seeds[i_data], x_probs, y[:-1, :])
        y_true = y[1:, :].argmax(dim=1)
        loss.append(weighted_ce_loss(y_pred, y_true))
        num_true, num_out = update_accuracy(y_pred, y_true, num_true, num_out)
        if i_pattern % batch_size == 0:
            loss = torch.cat(loss).mean()
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
            acc = (num_true / num_out).item()
            pbar.set_postfix_str(f"loss={loss.item():.4f}, acc={acc:.4f}")
            loss_history.append(loss.item())
            acc_history.append(acc)
            loss = []
        i_pattern += 1
        if i_pattern == 100:
            break
    if val_data is None:
        return loss_history, acc_history, (None, None)
    val_loss, val_true, val_out = [], 0, 0
    val_seeds = ext.encode_seeds(val_data["seeds"])
    with torch.no_grad():
        for x, y_val in val_data["data"]:
            y_pred = model(val_seeds, x, y_val[:-1, :])
            y_true = y_val[1:, :].argmax(dim=1)
            val_loss.append(weighted_ce_loss(y_pred, y_true))
            val_true, val_out = update_accuracy(y_pred, y_true, val_true, val_out)
    val_loss = torch.cat(val_loss).mean().item()
    val_acc = (val_true / val_out).item()
    print(f"Validation loss = {val_loss:.4f}; accuracy = {val_acc:.4f}")
    return loss_history, acc_history, (val_loss, val_acc)


def main():
    args = parse_args()
    print("Loading datasets...")
    datasets, max_indels, num_seeds = load_data(args.d)
    val_data = torch.load(args.v, weights_only=True) if args.v else None
    model = Model(num_seeds, max_indels, args.h)
    optimizer = torch.optim.AdamW(model.parameters())
    checkpoint_path = args.o + "_checkpoint.pt"
    if os.path.isfile(checkpoint_path):
        checkpoint = torch.load(checkpoint_path, weights_only=True)
        model.load_state_dict(checkpoint["model_state_dict"])
        optimizer.load_state_dict(checkpoint["optimizer_state_dict"])
        print("Model state loaded from checkpoint")
    model.summary()
    torch.set_num_threads(args.t)
    print(f"Maximum indel length: {max_indels}")
    print(f"Number of seeds: {num_seeds}")
    print(f"Number of training samples: {sum(len(d['data']) for d in datasets)}")
    print(f"Number of validation samples: {len(val_data['data']) if val_data else 0}")
    print(f"Using {torch.get_num_threads()} threads")
    history = {"loss": [], "acc": []}
    if val_data:
        history.update({"val_loss": [], "val_acc": []})
    for i_epoch in range(args.e):
        loss, acc, val = train(model, optimizer, datasets, args.b, i_epoch, val_data)
        history["loss"].extend(loss)
        history["acc"].extend(acc)
        if val_data:
            history["val_loss"].append(val[0])
            history["val_acc"].append(val[1])
        state_dict = dict()
        state_dict["model_state_dict"] = model.state_dict()
        state_dict["optimizer_state_dict"] = optimizer.state_dict()
        torch.save(state_dict, checkpoint_path)
        history_file = open(args.o + "_history.json", "w")
        json.dump(history, history_file)
        history_file.close()
    torch.jit.script(model).save(args.o + "_script.pt")


if __name__ == "__main__":
    main()
