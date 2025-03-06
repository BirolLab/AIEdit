import random

import torch
import torchinfo

from aiedit import core


class EditModel(torch.nn.Module):

    EDIT_ENCODER_INDEX = {
        core.EditType.SUBSTITUTE: 1,
        core.EditType.INSERT: 2,
        core.EditType.DELETE: 3,
    }

    def __init__(self, num_seeds: int, model_dim: int, exploration_factor: float):
        super().__init__()
        self._num_seeds = num_seeds
        self._model_dim = model_dim
        self.exploration_factor = exploration_factor
        self.seeds_encoder = torch.nn.GRU(num_seeds, model_dim)
        self.state_encoder = torch.nn.GRU(num_seeds + 1, model_dim)
        self.edits_encoder = torch.nn.GRU(4, model_dim)
        self.out = torch.nn.Linear(3 * model_dim + 4, core.ModelInterface.NUM_OUTPUTS)

    @staticmethod
    def get_edit_index(edit: core.Edit):
        if edit is None:
            return 0
        return EditModel.EDIT_ENCODER_INDEX[edit.type]

    def make_copy(self):
        model = EditModel(self._num_seeds, self._model_dim, self.exploration_factor)
        for target_param, param in zip(model.parameters(), self.parameters()):
            target_param.data.copy_(param)
        return model

    def forward(self, x_seeds, x_state, x_edits, x_next):
        _, h_seeds = self.seeds_encoder(x_seeds)
        _, h_state = self.state_encoder(x_state)
        _, h_edits = self.edits_encoder(x_edits)
        x_hidden = torch.cat([h_seeds, h_state, h_edits, x_next], dim=-1)
        return self.out(x_hidden)

    def select_edits(self, y_model: torch.FloatTensor) -> torch.LongTensor:
        if random.random() < self.exploration_factor:
            rand_high = core.ModelInterface.NUM_OUTPUTS - 1
            return torch.randint(0, rand_high, (y_model.size(0),))
        else:
            return y_model.argmax(dim=-1)

    def summary(self) -> None:
        x_seeds = torch.empty(30, self._num_seeds)
        x_state = torch.empty(32, self._num_seeds + 1)
        x_edits = torch.empty(5, 4)
        x_next = torch.empty(1, 4)
        data = [x_seeds, x_state, x_edits, x_next]
        torchinfo.summary(self, input_data=data, col_width=15)
