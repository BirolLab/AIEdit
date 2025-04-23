import os
import unittest

import aiedit
import aiedit.utils
import btllib
import numpy as np


class TestEditRegionFinder(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        data_path = os.path.join(os.path.dirname(__file__), "data")
        cbf_path = os.path.join(data_path, "counts.cbf")
        hist_path = os.path.join(data_path, "probs.tsv")
        seeds = aiedit.utils.load_seeds(os.path.join(data_path, "seeds.txt"))
        cls.kmer_model = aiedit.core.CBFKmerModel(cbf_path, hist_path, seeds)
        assembly_path = os.path.join(data_path, "assembly.fa")
        sr = btllib.SeqReader(assembly_path, btllib.SeqReaderFlag.LONG_MODE)
        cls.seq = next(iter(sr)).seq

    def test_edit_region_finder(self):
        hits = []
        hash_fn = btllib.NtHash(
            self.__class__.seq,
            self.__class__.kmer_model.get_num_hashes(),
            self.__class__.kmer_model.get_kmer_size(),
        )
        while hash_fn.roll():
            hashes = np.array(hash_fn.hashes(), dtype=np.uint64)
            hits.append(self.__class__.kmer_model.score(hashes) >= 0.5)
        diffs = np.diff(np.array(hits).astype(int))
        start_positions = set(np.where(diffs == -1)[0] + 1)

        erf = aiedit.core.EditRegionFinder(self.__class__.seq, self.__class__.kmer_model, 0.5, 10)
        for region in erf:
            self.assertIn(region[0], start_positions)
