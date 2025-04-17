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
        seeds = aiedit.utils.load_seeds(os.path.join(data_path, "seeds.txt"))
        cls.kmer_model = aiedit.core.CBFKmerModel(cbf_path, hist_path, seeds)

    def test_reference_kmer(self):
        kmer = "GTGGCTCCCGGTACCTAGGCGCTGA"
        nthash = btllib.NtHash(
            kmer, self.__class__.kmer_model.get_num_hashes(), len(kmer)
        )
        nthash.roll()
        hashes = np.array(nthash.hashes(), dtype=np.uint64)
        self.assertGreaterEqual(self.__class__.kmer_model.score(hashes), 0.5)

    def test_error_kmer(self):
        kmer = "ATGGCTCCCGGTACCTAGGCGCTGA"
        nthash = btllib.NtHash(
            kmer, self.__class__.kmer_model.get_num_hashes(), len(kmer)
        )
        nthash.roll()
        hashes = np.array(nthash.hashes(), dtype=np.uint64)
        self.assertLess(self.__class__.kmer_model.score(hashes), 0.5)
