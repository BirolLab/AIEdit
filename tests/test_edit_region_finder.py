import os
import unittest

import aiedit
import btllib
import numpy as np


class TestEditRegionFinder(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        data_path = os.path.join(os.path.dirname(__file__), "data")
        kmers_path = os.path.join(data_path, "kmers.bf")
        seeds_path = os.path.join(data_path, "seeds.bf")
        cls.kmer_model = aiedit.core.BFKmerModel(seeds_path, kmers_path)
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
