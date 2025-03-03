import aiedit
import torch
import torchinfo


class Model(torch.nn.Module):

    def __init__(self, num_seeds: int, model_dim: int):
        super().__init__()
        self._num_seeds = num_seeds
        self.seeds_encoder = torch.nn.GRU(num_seeds, model_dim)
        self.state_encoder = torch.nn.GRU(num_seeds + 1, model_dim)
        self.edits_encoder = torch.nn.GRU(aiedit.ModelInterface.NUM_OUTPUTS, model_dim)
        self.out = torch.nn.Linear(3 * model_dim + 4, aiedit.ModelInterface.NUM_OUTPUTS)

    def forward(self, x_seeds, x_state, x_edits, x_next):
        _, h_seeds = self.seeds_encoder(x_seeds)
        _, h_state = self.state_encoder(x_state)
        _, h_edits = self.edits_encoder(x_edits)
        x_hidden = torch.cat([h_seeds, h_state, h_edits, x_next], dim=-1)
        return self.out(x_hidden)

    def summary(self):
        x_seeds = torch.empty(30, self._num_seeds)
        x_state = torch.empty(32, self._num_seeds + 1)
        x_edits = torch.empty(5, aiedit.ModelInterface.NUM_OUTPUTS)
        x_next = torch.empty(1, 4)
        torchinfo.summary(self, input_data=[x_seeds, x_state, x_edits, x_next])
