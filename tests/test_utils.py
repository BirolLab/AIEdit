import unittest

import aiedit


class TestUtils(unittest.TestCase):

    def test_apply_edits(self):
        seq = "ACGGTGCAGTC"
        edits = [(1, 1, "+G"), (4, 1, "C**"), (8, 1, "-")]
        expected = "AGCGGCGCATC"
        self.assertEqual(aiedit.core.apply_edits(seq, edits), expected)
