import os
import unittest

import aiedit
import btllib
import numpy as np


class TestKmerModel(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        data_path = os.path.join(os.path.dirname(__file__), "data")
        cbf_path = os.path.join(data_path, "counts.cbf")
        hist_path = os.path.join(data_path, "probs.tsv")
        cls.seeds_path = os.path.join(data_path, "seeds.txt")
        cls.kmer_model = aiedit.KmerModel(cbf_path, hist_path, cls.seeds_path, 0.5)

    def test_seeds(self):
        with open(self.__class__.seeds_path) as fp:
            seeds = list(map(str.strip, fp.readlines()))
        self.assertListEqual(self.__class__.kmer_model.seeds, seeds)

    def test_reference_kmer(self):
        kmer = "GTGGCTCCCGGTACCTAGGCGCTGA"
        nthash = btllib.NtHash(kmer, self.__class__.kmer_model.num_hashes, len(kmer))
        nthash.roll()
        hashes = np.array(nthash.hashes(), dtype=np.uint64)
        self.assertTrue(self.__class__.kmer_model.is_hit(hashes))

    def test_error_kmer(self):
        kmer = "ATGGCTCCCGGTACCTAGGCGCTGA"
        nthash = btllib.NtHash(kmer, self.__class__.kmer_model.num_hashes, len(kmer))
        nthash.roll()
        hashes = np.array(nthash.hashes(), dtype=np.uint64)
        self.assertFalse(self.__class__.kmer_model.is_hit(hashes))
