import math
import typing

import numpy as np
import torch

from aiedit import core, edit_model, utils


class ModelTrainer:

    def __init__(
        self,
        model: edit_model.EditModel,
        optimizer: torch.optim.Optimizer,
        kmer_model: core.KmerModel,
        hit_threshold: float,
        max_edits: int,
        checkpoint_frequency: int,
    ):
        self._model = model
        self._optimizer = optimizer
        self._target_net = model.make_copy().eval()
        self._kmer_model = kmer_model
        self._hit_threshold = hit_threshold
        self._max_edits = max_edits
        self._checkpoint_frequency = checkpoint_frequency
        self._loss_history, self._reward_history = [], []
        self._num_steps = 0
        x_seeds = core.ModelInterface.encode_seeds(kmer_model.seeds)
        self._x_seeds = utils.buffer2d_to_tensor(x_seeds).clone()

    def _update_target_net(self, tau: float):
        zipped_params = zip(self._target_net.parameters(), self._model.parameters())
        for tgt_param, param in zipped_params:
            tgt_param.data.copy_(tau * param + (1 - tau) * tgt_param)

    def _calculate_reward(
        self,
        interface: core.ModelInterface,
        initial_signature: torch.FloatTensor,
        fixed: bool,
    ) -> float:
        if not interface.is_terminated() and not fixed:
            return 0.0
        edits_reward = interface.num_edits_left / self._max_edits
        if fixed:
            return edits_reward + 1.0
        signature = interface.get_signature()
        signature_arr = np.array(signature, copy=False)
        final_err = signature_arr.mean() if signature.num_rows > 0 else 0
        initial_err = initial_signature.mean().item()
        if final_err >= initial_err:
            return -1.0
        return initial_err - final_err + edits_reward

    @torch.no_grad()
    def _get_target_value(
        self,
        interface: core.ModelInterface,
        fixed: bool,
        signature: torch.FloatTensor,
        x_edits: torch.FloatTensor,
    ) -> torch.FloatTensor:
        if interface.is_terminated() or fixed:
            return torch.zeros(1)
        x_probs = torch.tensor(interface.get_next_probs()).unsqueeze(0)
        y_tgt = self._target_net(self._x_seeds, signature, x_edits, x_probs)
        return y_tgt.amax(dim=-1)

    def _train_on_region(self, seq: str, region: tuple[int]) -> tuple[float, float]:
        losses, reward = [], 0
        interface = core.ModelInterface(seq, *region, self._max_edits, self._kmer_model)
        signature = interface.get_signature()
        x_sig = utils.buffer2d_to_tensor(signature)
        x_edits = torch.zeros(self._max_edits, 4)
        fixed = False
        while not interface.is_terminated() and not fixed:
            self._model.exploration_factor = 0
            x_probs = torch.tensor(interface.get_next_probs()).unsqueeze(0)
            q_vals = self._model(self._x_seeds, x_sig, x_edits.clone(), x_probs)
            edit = self._model.select_edits(q_vals).item()
            applied_edit = interface.update(edit)
            i_edit = edit_model.EditModel.get_edit_index(applied_edit)
            x_edits[self._max_edits - interface.num_edits_left - 1, i_edit] = 1.0
            kmer_probs = interface.get_kmer_probs()
            fixed = all(p < self._hit_threshold for p in kmer_probs)
            reward = self._calculate_reward(interface, x_sig, fixed)
            target = reward + self._get_target_value(interface, fixed, x_sig, x_edits)
            loss = torch.nn.functional.mse_loss(q_vals[:, edit], target)
            losses.append(loss.item())
            self._optimizer.zero_grad()
            loss.backward()
            self._optimizer.step()
            self._num_steps += 1
        self._update_target_net(1 - math.exp(-0.0001 * self._num_steps))
        final_loss = sum(losses) / len(losses) if len(losses) > 0 else 0
        return final_loss, reward

    def _region_is_valid(self, region: tuple[int]) -> bool:
        n_kmers = region[1] - region[0]
        min_kmers = self._max_edits
        max_kmers = self._max_edits + self._kmer_model.kmer_size
        return min_kmers <= n_kmers <= max_kmers

    def train(
        self,
        seq: str,
        log_callback: typing.Callable,
        checkpoint_callback: typing.Callable,
    ):
        edit_finder = core.EditRegionFinder(seq, self._kmer_model, self._hit_threshold)
        valid_regions = filter(self._region_is_valid, edit_finder)
        for region in valid_regions:
            epsilon = self._model.exploration_factor
            loss1, _ = self._train_on_region(seq, region)
            self._model.exploration_factor = 0.0
            loss2, reward = self._train_on_region(seq, region)
            self._model.exploration_factor = epsilon
            loss = (loss1 + loss2) / 2
            log_callback(region, loss, reward, self._num_steps)
            self._loss_history.append(loss)
            self._reward_history.append(reward)
            if len(self._reward_history) % self._checkpoint_frequency == 0:
                checkpoint_callback(self._loss_history, self._reward_history)
        checkpoint_callback(self._loss_history, self._reward_history)
