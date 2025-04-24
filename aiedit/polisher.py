import functools

import torch
import torch.multiprocessing as mp

from aiedit import core, utils
from aiedit.model import Model


def _set_globals(km):
    global kmer_model
    kmer_model = km


@torch.no_grad()
def _get_edits(seq: str, model: Model, x_seeds: torch.FloatTensor, region: tuple[int]):
    global kmer_model
    start_pos = region[0] + kmer_model.get_kmer_size() - 1
    interface = core.ModelInterface(seq, *region, model._max_indels, kmer_model)
    x_sig = utils.buffer2d_to_tensor(interface.get_signature())
    y_pred = model(x_seeds, x_sig)
    y_pred = [y.squeeze(0) for y in y_pred]
    outputs, sizes = [y.data_ptr() for y in y_pred], [y.size(0) for y in y_pred]
    results = interface.update(outputs, sizes)
    return (start_pos, region[1] - start_pos, *results)


class Polisher:

    def __init__(
        self, model: Model, kmer_model, num_threads: int, score_threshold: float
    ):
        self._model = model
        self._kmer_model = kmer_model
        self._score_threshold = score_threshold
        self._pool = mp.Pool(num_threads, _set_globals, (kmer_model,))
        self._x_seeds = utils.encode_seeds(kmer_model.get_seeds()).share_memory_()

    def polish(self, seq: str):
        edits = []
        regions = core.EditRegionFinder(
            seq, self._kmer_model, self._score_threshold, self._model._max_mismatches
        )
        func = functools.partial(_get_edits, seq, self._model, self._x_seeds)
        for result in self._pool.imap(func, regions, 1000):
            edits.append(result)
        return edits
