import functools
import os
import sys
import threading

import btllib
import torch
import torch.multiprocessing as mp

from aiedit import core, utils
from aiedit.edit_model import EditModel

EDITS = ["sub", "ins", "del"]


def _init_pool(km: core.KmerModel):
    global kmer_model
    kmer_model = km


@torch.no_grad
def _polish(
    seq: str,
    model: EditModel,
    x_seeds: torch.FloatTensor,
    hit_threshold: float,
    max_edits: int,
    edits,
):
    global kmer_model
    edit_finder = core.EditRegionFinder(seq, kmer_model, hit_threshold)
    for region in edit_finder:
        edits_found = []
        interface = core.ModelInterface(seq, *region, max_edits, kmer_model)
        signature = interface.get_signature()
        x_sig = utils.buffer2d_to_tensor(signature)
        x_edits = torch.zeros(max_edits, 4)
        while not interface.is_terminated():
            x_probs = torch.tensor(interface.get_next_probs()).unsqueeze(0)
            q_vals = model(x_seeds, x_sig, x_edits.clone(), x_probs)
            edit = q_vals.argmax(dim=-1).item()
            applied_edit = interface.update(edit)
            i_edit = EditModel.get_edit_index(applied_edit)
            x_edits[max_edits - interface.num_edits_left - 1, i_edit] = 1.0
            if applied_edit is not None:
                row = (applied_edit.position, EDITS[i_edit - 1], applied_edit.new_base)
                edits_found.append(row)
            if all(p < hit_threshold for p in interface.get_kmer_probs()):
                edits.extend(edits_found)
                interface.terminate()


class SequencePolisher:

    def __init__(
        self,
        kmer_model: core.KmerModel,
        model: EditModel,
        hit_threshold: float,
        max_edits: int,
        num_threads: int,
        contig_mode: bool,
        out_prefix: str,
    ):
        self._model = model.share_memory()
        self._hit_threshold = hit_threshold
        self._max_edits = max_edits
        self._num_threads = num_threads
        self._contig_mode = contig_mode
        self._pool = mp.Pool(num_threads, _init_pool, (kmer_model,))
        self._manager = mp.Manager()
        self._queue_blocker = threading.Semaphore(num_threads)
        encoded_seeds = core.ModelInterface.encode_seeds(kmer_model.seeds)
        self._x_seeds = utils.buffer2d_to_tensor(encoded_seeds).clone().share_memory_()
        self._out_prefix = out_prefix
        self._edits_file_lock = mp.Lock()
        self._seq_writer = btllib.SeqWriter(out_prefix + "polished.fa")

    def _callback(self, seq, seq_id, seq_comment, edits, _):
        self._queue_blocker.release()
        print(f"[{seq_id}] Found {len(edits)} edits")
        with self._edits_file_lock:
            with open(self._out_prefix + "edits.tsv", "a") as fp:
                for edit in edits:
                    row = seq_id + "\t" + "\t".join(map(str, edit))
                    fp.write(row + os.linesep)
        self._seq_writer.write(seq_id, seq_comment, core.apply_edits(seq, list(edits)))

    def _error_callback(self, seq_id, error):
        self._queue_blocker.release()
        print(f"[{seq_id}] Error:", error, file=sys.stderr)

    def queue_sequence(self, seq_id: str, seq: str, seq_comment: str):
        edits = self._manager.list()
        args = (
            seq,
            self._model,
            self._x_seeds,
            self._hit_threshold,
            self._max_edits,
            edits,
        )
        callback = functools.partial(self._callback, seq, seq_id, seq_comment, edits)
        error_callback = functools.partial(self._error_callback, seq_id)
        self._queue_blocker.acquire()
        print(f"[{seq_id}] Polishing started ({len(seq):,}bp)")
        self._pool.apply_async(_polish, args, {}, callback, error_callback)

    def close(self):
        self._pool.close()
        self._pool.join()
