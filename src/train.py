import argparse
import os

import aiedit_torch_extensions as ext
import torch
import torchinfo
import tqdm


class Model(torch.nn.Module):

    def __init__(self, num_seeds: int, max_indels: int, hidden_dim: int = 32):
        super(Model, self).__init__()
        model_dim = num_seeds + 2 * max_indels + 1
        self.__pos_enc = ext.positional_encoding(1000, hidden_dim)
        self.register_buffer("pos_enc", self.__pos_enc)
        self.__probs_proj = torch.nn.Linear(model_dim, hidden_dim)
        self.__seeds_proj = torch.nn.Linear(num_seeds, hidden_dim)
        self.__edits_proj = torch.nn.Linear(5, hidden_dim)
        self.__seeds2probs = torch.nn.Transformer(
            hidden_dim, 4, 1, 1, hidden_dim, batch_first=True
        )
        self.__probs2edits = torch.nn.Transformer(
            hidden_dim, 4, 1, 1, hidden_dim, batch_first=True
        )
        self.__out = torch.nn.Linear(hidden_dim, 5)
        self.__input_sizes = [(100, model_dim), (25, num_seeds), (max_indels, 5)]

    def summary(self):
        torchinfo.summary(self, input_size=self.__input_sizes)

    def forward(self, x_probs, x_seeds, x_edits):
        x_probs = self.__probs_proj(x_probs) + self.__pos_enc[: x_probs.size(0), :]
        x_seeds = self.__seeds_proj(x_seeds)
        x_edits = self.__edits_proj(x_edits) + self.__pos_enc[: x_edits.size(0), :]
        x_probs = self.__seeds2probs(x_seeds, x_probs)
        mask = torch.ones(x_edits.size(0), x_edits.size(0))
        mask = torch.triu(mask * float("-inf"), diagonal=1)
        y = self.__probs2edits(x_probs, x_edits, tgt_mask=mask, tgt_is_causal=True)
        return self.__out(y)


def parse_args():
    default_t = torch.get_num_threads()
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", help="path to dataset", required=True)
    parser.add_argument("-n", help="number of epochs", type=int, default=5)
    parser.add_argument("-t", help="number of threads", type=int, default=default_t)
    parser.add_argument("-o", help="path to model checkpoint file", required=True)
    return parser.parse_args()


def weighted_ce_loss(logits, targets, reduction_factor=0.01):
    ce_loss = torch.nn.functional.cross_entropy(logits, targets, reduction="none")
    ce_loss[(logits.argmax(dim=1) == targets) & (targets == 0)] *= reduction_factor
    return ce_loss.mean()


def train(model, optimizer, data, x_seeds, i_epoch):
    epoch_loss = 0
    pbar = tqdm.tqdm(data, unit="patterns", desc=f"Epoch {i_epoch + 1}")
    i_pattern = 1
    for x, y in pbar:
        optimizer.zero_grad()
        y_pred = model(x, x_seeds, y)[:-1, :]
        loss = weighted_ce_loss(y_pred, y[1:, :].argmax(dim=1))
        loss.backward()
        optimizer.step()
        epoch_loss += loss.item()
        pbar.set_postfix_str(f"loss={epoch_loss/i_pattern:.4f}")
        i_pattern += 1


def main():
    args = parse_args()
    print("Loading dataset...")
    dataset = torch.load(args.d, weights_only=True)
    data, seeds, max_indels = dataset["data"], dataset["seeds"], dataset["max_indels"]
    print(f"Number of patterns: {len(data)}")
    print(f"Maximum indel length: {max_indels}")
    print("Seeds:", *seeds, sep=os.linesep)
    model = Model(len(seeds), max_indels)
    optimizer = torch.optim.AdamW(model.parameters())
    if os.path.isfile(args.o):
        print("Loading model checkpoint")
        checkpoint = torch.load(args.o, weights_only=True)
        model.load_state_dict(checkpoint["model_state_dict"])
        optimizer.load_state_dict(checkpoint["optimizer_state_dict"])
    model.summary()
    torch.set_num_threads(args.t)
    print(f"Using {torch.get_num_threads()} threads")
    x_seeds = ext.encode_seeds(seeds)
    x_seeds += ext.positional_encoding(len(seeds[0]), len(seeds))
    for i_epoch in range(args.n):
        train(model, optimizer, data, x_seeds, i_epoch)
        state_dict = {
            "model_state_dict": model.state_dict(),
            "optimizer_state_dict": optimizer.state_dict(),
        }
        torch.save(state_dict, args.o)


if __name__ == "__main__":
    main()
