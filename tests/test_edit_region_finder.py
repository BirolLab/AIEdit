import os
import unittest

import aiedit
import btllib
import numpy as np


class TestEditRegionFinder(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        data_path = os.path.join(os.path.dirname(__file__), "data")
        cbf_path = os.path.join(data_path, "counts.cbf")
        hist_path = os.path.join(data_path, "probs.tsv")
        seeds_path = os.path.join(data_path, "seeds.txt")
        cls.kmer_model = aiedit.KmerModel(cbf_path, hist_path, seeds_path, 0.5)
        assembly_path = os.path.join(data_path, "assembly.fa")
        sr = btllib.SeqReader(assembly_path, btllib.SeqReaderFlag.LONG_MODE)
        cls.seq = next(iter(sr)).seq

    def test_edit_region_finder(self):
        hits = []
        hash_fn = btllib.NtHash(
            self.__class__.seq,
            self.__class__.kmer_model.num_hashes,
            self.__class__.kmer_model.kmer_size,
        )
        while hash_fn.roll():
            hashes = np.array(hash_fn.hashes(), dtype=np.uint64)
            hits.append(self.__class__.kmer_model.is_hit(hashes))
        diffs = np.diff(np.array(hits).astype(int))
        start_positions = np.where(diffs == -1)[0] + 1
        end_positions = np.where(diffs == 1)[0] + 1
        expected_regions = set(zip(start_positions, end_positions))

        erf = aiedit.EditRegionFinder(self.__class__.seq, self.__class__.kmer_model)
        for region in erf:
            self.assertIn(region, expected_regions)
