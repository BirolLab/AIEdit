import functools
import threading

import torch
import torch.multiprocessing as mp

from aiedit import core, utils
from aiedit.model import Model


def _set_globals(km):
    global kmer_model
    kmer_model = km


@torch.no_grad
def _get_edits(seq: str, model: Model, x_seeds: torch.FloatTensor, region: tuple[int]):
    global kmer_model
    interface = core.ModelInterface(seq, *region, model.max_edits, kmer_model)
    x_sig = utils.buffer2d_to_tensor(interface.get_signature())
    model_output = model(x_seeds, x_sig)
    indel_prob = model_output[0].item()
    mis_pattern = model_output[1].data_ptr()
    num_indels = model_output[2].argmax().item()
    start_pos = region[0] + kmer_model.get_kmer_size()
    return (start_pos, interface.update(indel_prob, mis_pattern, num_indels))


class Polisher:

    def __init__(self, model: Model, kmer_model, num_threads: int):
        self._model = model
        self._kmer_model = kmer_model
        self._pool = mp.Pool(num_threads, _set_globals, (kmer_model,))
        self._x_seeds = utils.encode_seeds(kmer_model.get_seeds()).share_memory_()

    def polish(self, seq: str):
        regions = core.EditRegionFinder(seq, self._kmer_model, 0.5)
        func = functools.partial(_get_edits, seq, self._model, self._x_seeds)
        for result in self._pool.imap(func, regions, 100):
            print(result)
