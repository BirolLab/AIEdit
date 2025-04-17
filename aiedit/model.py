import torch
import torchinfo


class Model(torch.nn.Module):

    def __init__(
        self, num_seeds: int, max_mismatches: int, max_indels: int, model_dim: int
    ):
        super().__init__()
        self._num_seeds = num_seeds
        self._max_mismatches = max_mismatches
        self._max_indels = max_indels
        self._model_dim = model_dim
        num_features = num_seeds * (max_indels + 1) + 1
        self.seeds_encoder = torch.nn.GRU(num_seeds, model_dim)
        self.signature_encoder = torch.nn.GRU(num_features, model_dim)
        self.indel_prob = torch.nn.Linear(model_dim * 2, 1)
        self.mismatches = torch.nn.Linear(model_dim * 2, max_mismatches)
        self.indels = torch.nn.Linear(model_dim * 2, max_indels * 2)

    @staticmethod
    def from_checkpoint(path: str):
        checkpoint = torch.load(path, weights_only=True)
        model_args = ["num_seeds", "max_mismatches", "max_indels", "model_dim"]
        args = (checkpoint[k].item() for k in model_args)
        model = Model(*args)
        model.load_state_dict(checkpoint["model"])
        return model, checkpoint["optimizer"]

    def save(self, path: str, optimizer_state_dict):
        checkpoint = {
            "num_seeds": torch.tensor(self._num_seeds, dtype=torch.int),
            "max_mismatches": torch.tensor(self._max_mismatches, dtype=torch.int),
            "max_indels": torch.tensor(self._max_indels, dtype=torch.int),
            "model_dim": torch.tensor(self._model_dim, dtype=torch.int),
        }
        checkpoint["model"] = self.state_dict()
        checkpoint["optimizer"] = optimizer_state_dict
        torch.save(checkpoint, path)

    def forward(self, x_seeds, x_signature):
        _, h_seeds = self.seeds_encoder(x_seeds)
        _, h_signature = self.signature_encoder(x_signature)
        hidden = torch.cat([h_seeds, h_signature], dim=-1)
        return self.indel_prob(hidden), self.mismatches(hidden), self.indels(hidden)

    def print_summary(self) -> None:
        input_shape = [
            (30, self._num_seeds),
            (32, self._num_seeds * (self._max_indels + 1) + 1),
        ]
        torchinfo.summary(self, input_shape, col_width=15)
