import os
import unittest

import btllib
import numpy as np

import aiedit.core
import aiedit.train.main


class TestModelInterface(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.data_path = os.path.join(os.path.dirname(__file__), "data")
        kmers_path = os.path.join(cls.data_path, "kmers.bf")
        seeds_path = os.path.join(cls.data_path, "seeds.bf")
        cls.kmer_model = aiedit.core.BFKmerModel(seeds_path, kmers_path)
        reference_path = os.path.join(cls.data_path, "reference.fa")
        sr = btllib.SeqReader(reference_path, btllib.SeqReaderFlag.LONG_MODE)
        cls.ref = next(iter(sr)).seq

    def test_repeats(self):
        k = self.__class__.kmer_model.get_kmer_size()
        rep = "AGGCTTTC"
        seq = "A" * k + rep + "CA" * k
        expected = np.array([1, 0, 1, 0, 0, 1, 1, 0], dtype=float)
        env = aiedit.core.ModelInterface(seq, 1, k + 1, 5, 5, self.__class__.kmer_model)
        repeats = np.array(env.get_signature(), copy=False)[: len(rep), 0]
        self.assertTrue(np.array_equal(repeats, expected))

    def test_substitution(self):
        subs = {"A": "C", "C": "T", "G": "A", "T": "G"}
        k = self.__class__.kmer_model.get_kmer_size()
        seq = self.__class__.ref
        seq = seq[:k] + subs[seq[k]] + seq[k + 1 :]
        env = aiedit.core.ModelInterface(seq, 1, k + 1, 5, 0, self.__class__.kmer_model)

        x_seeds = aiedit.train.data.encode_seeds(self.__class__.kmer_model.get_seeds())
        signature = np.array(env.get_signature(), copy=False)
        self.assertTrue(np.array_equal(signature[:, 1:].round(), 1 - x_seeds))

        edit_type, applied_edit, *_ = env.update(0)
        self.assertEqual(edit_type, aiedit.core.EditType.SUBSTITUTE)
        self.assertEqual(applied_edit, self.__class__.ref[k])

    def test_deletion(self):
        k = self.__class__.kmer_model.get_kmer_size()
        seq = self.__class__.ref
        seq = seq[:k] + "C" + seq[k:]
        env = aiedit.core.ModelInterface(seq, 1, k + 1, 5, 5, self.__class__.kmer_model)

        edit_type, applied_edit, _ = env.update(16)
        self.assertEqual(edit_type, aiedit.core.EditType.DELETE)
        self.assertEqual(applied_edit, "-")

    def test_insertion(self):
        k = self.__class__.kmer_model.get_kmer_size()
        seq = self.__class__.ref
        seq = seq[:k] + seq[k + 1 :]
        env = aiedit.core.ModelInterface(seq, 1, k + 1, 5, 5, self.__class__.kmer_model)

        edit_type, applied_edit, *_ = env.update(21)
        self.assertEqual(edit_type, aiedit.core.EditType.INSERT)
        self.assertEqual(applied_edit, self.__class__.ref[k])
