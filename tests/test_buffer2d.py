import unittest

import aiedit
import numpy as np
import torch


class TestBuffer2D(unittest.TestCase):

    def setUp(self):
        self.raw_data = [[0.4, 0.2], [0.1, 0.7], [0.4, 0.1], [0.3, 0.8], [0.9, 0.2]]
        self.buffer = aiedit.Buffer2D(len(self.raw_data), len(self.raw_data[0]))
        for i in range(self.buffer.num_rows):
            for j in range(self.buffer.num_cols):
                self.buffer[i, j] = self.raw_data[i][j]

    def test_getter(self):
        for i in range(self.buffer.num_rows):
            for j in range(self.buffer.num_cols):
                self.assertAlmostEqual(self.buffer[i, j], self.raw_data[i][j])

    def test_as_numpy_array(self):
        buffer_arr = np.array(self.buffer, copy=False)
        data_arr = np.array(self.raw_data)
        self.assertEqual(buffer_arr.shape, data_arr.shape)
        self.assertTrue(np.allclose(buffer_arr, data_arr))

    def test_as_torch_tensor(self):
        buffer_tensor = torch.from_numpy(np.array(self.buffer, copy=False))
        data_tensor = torch.tensor(self.raw_data)
        self.assertEqual(buffer_tensor.size(), data_tensor.size())
        self.assertTrue(torch.allclose(buffer_tensor, data_tensor))
