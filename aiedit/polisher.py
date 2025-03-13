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
def _get_edits(seq: str, region):
    global kmer_model
    print(kmer_model.get_kmer_size(), region)


class Polisher:

    def __init__(self, model: Model, kmer_model, num_threads: int):
        self._model = model
        self._kmer_model = kmer_model
        self._pool = mp.Pool(num_threads, _set_globals, (kmer_model,))
        self._x_seeds = utils.encode_seeds(kmer_model.get_seeds()).share_memory_()

    def polish(self, seq: str):
        regions = core.EditRegionFinder(seq, self._kmer_model, 0.5)
        proc_func = functools.partial(_get_edits, seq)
        self._pool.imap(proc_func, regions)
