import os
import unittest

import btllib
import numpy as np

import aiedit


class TestKmerModel(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        data_path = os.path.join(os.path.dirname(__file__), "data")
        kmers_path = os.path.join(data_path, "kmers.bf")
        seeds_path = os.path.join(data_path, "seeds.bf")
        cls.kmer_model = aiedit.core.BFKmerModel(seeds_path, kmers_path)
        reference_path = os.path.join(data_path, "reference.fa")
        with btllib.SeqReader(reference_path, btllib.SeqReaderFlag.SHORT_MODE) as sr:
            cls.reference = next(iter(sr)).seq

    def test_reference_kmer(self):
        kmer = self.__class__.reference[: self.__class__.kmer_model.get_kmer_size()]
        nthash = btllib.NtHash(
            kmer, self.__class__.kmer_model.get_num_hashes(), len(kmer)
        )
        nthash.roll()
        hashes = np.array(nthash.hashes(), dtype=np.uint64)
        self.assertGreaterEqual(self.__class__.kmer_model.score(hashes), 0.5)

    def test_error_kmer(self):
        kmer = "A" * self.__class__.kmer_model.get_kmer_size()
        nthash = btllib.NtHash(
            kmer, self.__class__.kmer_model.get_num_hashes(), len(kmer)
        )
        nthash.roll()
        hashes = np.array(nthash.hashes(), dtype=np.uint64)
        self.assertLess(self.__class__.kmer_model.score(hashes), 0.5)
