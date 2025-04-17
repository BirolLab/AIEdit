import unittest

import torch

import aiedit.data
import aiedit.utils


class TestData(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.seq = "AGGTCATCATGG"
        cls.seeds = ["1110111", "1100011"]
        cls.kmer_model = aiedit.data._make_kmer_model(cls.seq, cls.seeds)

    def test_mismatch(self):
        seq, kmer_model = self.__class__.seq, self.__class__.kmer_model
        x_sig = aiedit.data._get_mismatch_signature(seq, "100", 0, kmer_model)
        x_seeds = aiedit.utils.encode_seeds(self.__class__.seeds)
        self.assertTrue(torch.equal(x_sig[:, 1:], 1 - x_seeds))
