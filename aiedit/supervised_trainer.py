import random

import numpy as np
import torch

from aiedit import core, utils
from aiedit.edit_model import EditModel


def _generate_edits(seq: str, max_edits: int, kmer_size: int) -> list[list[core.Edit]]:
    edits = []
    for i in range(0, 2 ** (max_edits - 1)):
        mis = []
        for pos, b in enumerate("1" + format(i, f"0{max_edits}b")):
            if b == "1":
                bases = list("ACGT")
                bases.remove(seq[kmer_size + pos])
                mis.append((kmer_size + pos, "sub", random.choice(bases)))
        edits.append(mis)
    for i in range(max_edits):
        edits.append([(kmer_size + j, "del", ".") for j in range(i)])
        ins = [(kmer_size, "ins", random.choice("ACGT")) for _ in range(i)]
        edits.append(ins)
    return edits


class SupervisedTrainer:

    def __init__(
        self,
        model: EditModel,
        optimizer: torch.optim.Optimizer,
        kmer_model: core.KmerModel,
        hit_threshold: float,
        max_edits: int,
    ):
        self._model = model
        self._optimizer = optimizer
        self._kmer_model = kmer_model
        self._hit_threshold = hit_threshold
        self._max_edits = max_edits

    def _get_clean_seq(self, seq):
        region = (self._kmer_model.kmer_size, len(seq) - self._kmer_model.kmer_size)
        interface = core.ModelInterface(seq, *region, self._max_edits, self._kmer_model)
        below = np.array(interface.get_kmer_probs()) < self._hit_threshold
        starts = np.where(np.diff(np.r_[0, below, 0]) == 1)[0]
        ends = np.where(np.diff(np.r_[0, below, 0]) == -1)[0]
        for i, j in zip(starts, ends):
            if j - i >= self._kmer_model.kmer_size * 3:
                return seq[i:j]
        raise RuntimeError("No clean region found in reference")

    def _train_on_edits(self, seq, region, edits):
        interface = core.ModelInterface(seq, *region, self._max_edits, self._kmer_model)
        signature = interface.get_signature()
        x_sig = utils.buffer2d_to_tensor(signature)
        x_edits = torch.zeros(self._max_edits, 4)
        print(edits)
        print(x_sig)

    def train(self, ref_seq: str):
        clean_seq = self._get_clean_seq(ref_seq)
        edits = _generate_edits(clean_seq, self._max_edits, self._kmer_model.kmer_size)
        k = self._kmer_model.kmer_size
        for edit_list in edits:
            seq = core.apply_edits(clean_seq, edit_list)
            region = (1, k + 1 + self._max_edits)
            self._train_on_edits(seq, region, edit_list)
            return
