import os
import unittest

import btllib
import numpy as np
import torch

import aiedit
import aiedit.utils


class TestModelInterface(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        data_path = os.path.join(os.path.dirname(__file__), "data")
        cbf_path = os.path.join(data_path, "counts.cbf")
        hist_path = os.path.join(data_path, "probs.tsv")
        cls.seeds = aiedit.utils.load_seeds(os.path.join(data_path, "seeds.txt"))
        cls.kmer_model = aiedit.core.CBFKmerModel(cbf_path, hist_path, cls.seeds)
        reference_path = os.path.join(data_path, "reference.fa")
        sr = btllib.SeqReader(reference_path, btllib.SeqReaderFlag.LONG_MODE)
        cls.ref = next(iter(sr)).seq

    def test_repeats(self):
        k = self.__class__.kmer_model.get_kmer_size()
        rep = "AGGCTTTC"
        seq = "A" * k + rep + "CA" * k
        expected = np.array([1, 0, 1, 0, 0, 1, 1, 0], dtype=float)
        env = aiedit.core.ModelInterface(seq, 1, k + 1, 5, self.__class__.kmer_model)
        repeats = np.array(env.get_signature(), copy=False)[: len(rep), 0]
        self.assertTrue(np.array_equal(repeats, expected))

    def test_substitution(self):
        subs = {"A": "C", "C": "T", "G": "A", "T": "G"}
        k = self.__class__.kmer_model.get_kmer_size()
        seq = self.__class__.ref
        seq = seq[:k] + subs[seq[k]] + seq[k + 1 :]
        env = aiedit.core.ModelInterface(seq, 1, k + 1, 0, self.__class__.kmer_model)

        x_seeds = aiedit.utils.encode_seeds(self.__class__.seeds)
        signature = np.array(env.get_signature(), copy=False)
        self.assertTrue(np.array_equal(signature[:, 1:], 1 - x_seeds))

        y_pred = (
            torch.tensor([-1.0]),
            torch.tensor([1.0, -1.0, -1.0, -1.0, -1.0]),
            torch.zeros(1),
        )
        outputs = [y.data_ptr() for y in y_pred]
        sizes = [y.size(0) for y in y_pred]

        applied_edit = env.update(outputs, sizes)
        self.assertEqual(applied_edit, self.__class__.ref[k] + "****")

        signature = np.array(env.get_signature(), copy=False)
        self.assertTrue(signature[:, 1].all())

    def test_insertion(self):
        k = self.__class__.kmer_model.get_kmer_size()
        seq = self.__class__.ref
        seq = seq[:k] + seq[k + 1 :]
        env = aiedit.core.ModelInterface(seq, 1, k + 1, 5, self.__class__.kmer_model)

        y_indel = torch.zeros(10)
        y_indel[0] = 1.0
        y_pred = (torch.tensor([1.0]), torch.zeros(1), y_indel)
        outputs = [y.data_ptr() for y in y_pred]
        sizes = [y.size(0) for y in y_pred]

        applied_edit = env.update(outputs, sizes)
        self.assertEqual(applied_edit, "+" + self.__class__.ref[k])

        signature = np.array(env.get_signature(), copy=False)
        self.assertTrue(signature[:, 1].all())

    def test_deletion(self):
        k = self.__class__.kmer_model.get_kmer_size()
        seq = self.__class__.ref
        seq = seq[:k] + "C" + seq[k:]
        env = aiedit.core.ModelInterface(seq, 1, k + 1, 5, self.__class__.kmer_model)

        y_indel = torch.zeros(10)
        y_indel[5] = 1.0
        y_pred = (torch.tensor([1.0]), torch.zeros(1), y_indel)
        outputs = [y.data_ptr() for y in y_pred]
        sizes = [y.size(0) for y in y_pred]

        applied_edit = env.update(outputs, sizes)
        self.assertEqual(applied_edit, "-")

        signature = np.array(env.get_signature(), copy=False)
        self.assertTrue(signature[:, 1].all())
