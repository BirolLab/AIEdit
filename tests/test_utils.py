import unittest

import aiedit


class TestUtils(unittest.TestCase):

    def test_apply_edits(self):
        seq = "ACGGTGCAGTC"
        edits = [(1, "ins", "A"), (4, "sub", "C"), (6, "del", ".")]
        expected = "AACGGCGAGTC"
        self.assertEqual(aiedit.core.apply_edits(seq, edits), expected)
