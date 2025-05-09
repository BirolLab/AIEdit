import unittest

import torch

import aiedit.train.data


class TestData(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.seq = "TACGGTACTGACGTCTAG"
        cls.seeds = ["1110111", "1101011"]
        cls.kmer_model = aiedit.train.data._make_kmer_model(cls.seq, cls.seeds)

    @unittest.skip("Unsuitable test data")
    def test_mismatch(self):
        seq, kmer_model = self.__class__.seq, self.__class__.kmer_model
        x_sig = aiedit.train.data._get_mismatch_signature(seq, "100", 0, kmer_model)
        print(x_sig)
        x_seeds = aiedit.train.data.encode_seeds(self.__class__.seeds)
        self.assertTrue(torch.equal(x_sig[:, 1:], 1 - x_seeds))

    @unittest.skip("Unsuitable test data")
    def test_deletion(self):
        seq, kmer_model = self.__class__.seq, self.__class__.kmer_model
        x_sig = aiedit.train.data._get_deletion_sample(seq, 2, 3, kmer_model)
        print(x_sig)

    @unittest.skip("Unsuitable test data")
    def test_insertion(self):
        seq, kmer_model = self.__class__.seq, self.__class__.kmer_model
        x_sig = aiedit.train.data._get_insertion_sample(seq, 1, 3, kmer_model)
        print(x_sig)
