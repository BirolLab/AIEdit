import itertools
import os
import unittest

import aiedit
import btllib
import numpy as np


class TestModelInterface(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        data_path = os.path.join(os.path.dirname(__file__), "data")
        cbf_path = os.path.join(data_path, "counts.cbf")
        hist_path = os.path.join(data_path, "probs.tsv")
        seeds_path = os.path.join(data_path, "seeds.txt")
        cls.kmer_model = aiedit.core.KmerModel(cbf_path, hist_path, seeds_path)
        reference_path = os.path.join(data_path, "reference.fa")
        sr = btllib.SeqReader(reference_path, btllib.SeqReaderFlag.LONG_MODE)
        cls.ref = next(iter(sr)).seq

    def test_repeats(self):
        k = self.__class__.kmer_model.kmer_size
        rep = "AGGCTTTC"
        seq = "A" * k + rep + "CA" * k
        expected = np.array([1, 0, 1, 0, 0, 1, 1, 0], dtype=float)
        env = aiedit.core.ModelInterface(seq, 1, k + 1, 5, self.__class__.kmer_model)
        repeats = np.array(env.get_signature(), copy=False)[: len(rep), 0]
        self.assertTrue(np.array_equal(repeats, expected))

    def test_substitution(self):
        subs = {"A": "C", "C": "T", "G": "A", "T": "G"}
        k = self.__class__.kmer_model.kmer_size
        seq = self.__class__.ref
        seq = seq[:k] + subs[seq[k]] + seq[k + 1 :]
        env = aiedit.core.ModelInterface(seq, 1, k + 1, 5, self.__class__.kmer_model)

        x_seeds = aiedit.core.ModelInterface.encode_seeds(self.__class__.kmer_model.seeds)
        signature = np.array(env.get_signature(), copy=False)
        self.assertTrue(np.array_equal(signature[:, 1:], x_seeds))

        next_probs = np.array(env.get_next_probs(), copy=False)
        true_probs = np.ones(4)
        true_probs["ACGT".index(self.__class__.ref[k])] = 0.0
        true_probs["ACGT".index(seq[k])] = -1.0
        self.assertTrue(np.array_equal(next_probs, true_probs))

        applied_edit = env.update("ACGT".index(self.__class__.ref[k]) + 1)
        self.assertEqual(applied_edit.position, k)
        self.assertEqual(applied_edit.type, aiedit.core.EditType.SUBSTITUTE)
        self.assertEqual(applied_edit.new_base, self.__class__.ref[k])

        signature = np.array(env.get_signature(), copy=False)
        self.assertFalse(signature[:, 1:].any())

    def test_insertion(self):
        k = self.__class__.kmer_model.kmer_size
        seq = self.__class__.ref
        seq = seq[:k] + seq[k + 1 :]
        env = aiedit.core.ModelInterface(seq, 1, k + 1, 5, self.__class__.kmer_model)

        next_probs = np.array(env.get_next_probs(), copy=False)
        true_probs = np.ones(4)
        true_probs["ACGT".index(self.__class__.ref[k])] = 0.0
        true_probs["ACGT".index(seq[k])] = -1.0
        self.assertTrue(np.array_equal(next_probs, true_probs))

        applied_edit = env.update("ACGT".index(self.__class__.ref[k]) + 5)
        self.assertEqual(applied_edit.position, k)
        self.assertEqual(applied_edit.type, aiedit.core.EditType.INSERT)
        self.assertEqual(applied_edit.new_base, self.__class__.ref[k])

        signature = np.array(env.get_signature(), copy=False)
        self.assertFalse(signature[:, 1:].any())

    def test_deletion(self):
        k = self.__class__.kmer_model.kmer_size
        seq = self.__class__.ref
        seq = seq[:k] + "C" + seq[k:]
        env = aiedit.core.ModelInterface(seq, 1, k + 1, 5, self.__class__.kmer_model)

        next_probs = np.array(env.get_next_probs(), copy=False)
        true_probs = np.ones(4)
        true_probs["ACGT".index(self.__class__.ref[k])] = 0.0
        true_probs["ACGT".index(seq[k])] = -1.0
        self.assertTrue(np.array_equal(next_probs, true_probs))

        applied_edit = env.update(9)
        self.assertEqual(applied_edit.position, k)
        self.assertEqual(applied_edit.type, aiedit.core.EditType.DELETE)

        signature = np.array(env.get_signature(), copy=False)
        self.assertFalse(signature[:, 1:].any())
