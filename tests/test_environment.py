import itertools
import os
import unittest

import aiedit
import btllib
import numpy as np


class TestEnvironment(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        data_path = os.path.join(os.path.dirname(__file__), "data")
        cbf_path = os.path.join(data_path, "counts.cbf")
        hist_path = os.path.join(data_path, "probs.tsv")
        seeds_path = os.path.join(data_path, "seeds.txt")
        cls.kmer_model = aiedit.KmerModel(cbf_path, hist_path, seeds_path, 0.5)
        reference_path = os.path.join(data_path, "reference.fa")
        sr = btllib.SeqReader(reference_path, btllib.SeqReaderFlag.LONG_MODE)
        cls.ref = next(iter(sr)).seq

    def test_substitution(self):
        subs = {"A": "C", "C": "T", "G": "A", "T": "G"}
        k = self.__class__.kmer_model.kmer_size
        seq = self.__class__.ref
        seq = seq[:k] + subs[seq[k]] + seq[k + 1 :]
        env = aiedit.Environment(seq, 1, k + 1, 5, self.__class__.kmer_model)

        signature = np.array(env.get_signature(), copy=False)
        seeds = self.__class__.kmer_model.seeds
        x_seeds = np.empty(shape=(len(seeds[0]), len(seeds)))
        indices = itertools.product(range(x_seeds.shape[0]), range(x_seeds.shape[1]))
        for i, j in indices:
            x_seeds[i][j] = float(seeds[j][i])
        self.assertTrue(np.array_equal(signature, x_seeds))

        next_probs = np.array(env.get_next_probs(), copy=False)
        true_probs = np.ones(4)
        true_probs["ACGT".index(self.__class__.ref[k])] = 0.0
        true_probs["ACGT".index(seq[k])] = -1.0
        self.assertTrue(np.array_equal(next_probs, true_probs))

        env.act(aiedit.EditType.SUBSTITUTE, self.__class__.ref[k])
        signature = np.array(env.get_signature(), copy=False)
        self.assertFalse(signature.any())

    def test_insertion(self):
        k = self.__class__.kmer_model.kmer_size
        seq = self.__class__.ref
        seq = seq[:k] + seq[k + 1 :]
        env = aiedit.Environment(seq, 1, k + 1, 5, self.__class__.kmer_model)

        next_probs = np.array(env.get_next_probs(), copy=False)
        true_probs = np.ones(4)
        true_probs["ACGT".index(self.__class__.ref[k])] = 0.0
        true_probs["ACGT".index(seq[k])] = -1.0
        self.assertTrue(np.array_equal(next_probs, true_probs))

        env.act(aiedit.EditType.INSERT, self.__class__.ref[k])
        signature = np.array(env.get_signature(), copy=False)
        self.assertFalse(signature.any())

    def test_deletion(self):
        k = self.__class__.kmer_model.kmer_size
        seq = self.__class__.ref
        seq = seq[:k] + "C" + seq[k:]
        env = aiedit.Environment(seq, 1, k + 1, 5, self.__class__.kmer_model)

        next_probs = np.array(env.get_next_probs(), copy=False)
        true_probs = np.ones(4)
        true_probs["ACGT".index(self.__class__.ref[k])] = 0.0
        true_probs["ACGT".index(seq[k])] = -1.0
        self.assertTrue(np.array_equal(next_probs, true_probs))

        env.act(aiedit.EditType.DELETE, ".")
        signature = np.array(env.get_signature(), copy=False)
        self.assertFalse(signature.any())
