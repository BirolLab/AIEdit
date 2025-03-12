import torch


class Model(torch.nn.Module):

    def __init__(self, num_seeds: int, max_edits: int, model_dim: int):
        super().__init__()
        self.seeds_encoder = torch.nn.GRU(num_seeds, model_dim)
        self.signature_encoder = torch.nn.GRU(num_seeds + 1, model_dim)
        self.indel_prob = torch.nn.Linear(2 * model_dim, 1)
        self.mismatches = torch.nn.Linear(2 * model_dim, max_edits)
        self.indels = torch.nn.Linear(2 * model_dim, 2 * max_edits)

    def forward(self, x_seeds, x_signature):
        _, h_seeds = self.seeds_encoder(x_seeds)
        _, h_signature = self.signature_encoder(x_signature)
        hidden = torch.cat([h_seeds, h_signature], dim=-1)
        return self.indel_prob(hidden), self.mismatches(hidden), self.indels(hidden)
