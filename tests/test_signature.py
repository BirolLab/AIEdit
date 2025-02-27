import unittest

import aiedit
import numpy as np
import torch


class TestSignature(unittest.TestCase):

    def setUp(self):
        self.raw_data = [[0.4, 0.2], [0.1, 0.7], [0.4, 0.1], [0.3, 0.8], [0.9, 0.2]]
        self.signature = aiedit.Signature(len(self.raw_data), len(self.raw_data[0]))
        for i in range(self.signature.length):
            for j in range(self.signature.num_seeds):
                self.signature[i, j] = self.raw_data[i][j]

    def test_getter(self):
        for i in range(self.signature.length):
            for j in range(self.signature.num_seeds):
                self.assertAlmostEqual(self.signature[i, j], self.raw_data[i][j])

    def test_as_numpy_array(self):
        signature_arr = np.array(self.signature, copy=False)
        data_arr = np.array(self.raw_data)
        self.assertEqual(signature_arr.shape, data_arr.shape)
        self.assertTrue(np.allclose(signature_arr, data_arr))

    def test_as_torch_tensor(self):
        signature_tensor = torch.from_numpy(np.array(self.signature, copy=False))
        data_tensor = torch.tensor(self.raw_data)
        self.assertEqual(signature_tensor.size(), data_tensor.size())
        self.assertTrue(torch.allclose(signature_tensor, data_tensor))
