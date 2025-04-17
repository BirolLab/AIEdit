import unittest

import aiedit


class TestUtils(unittest.TestCase):

    def test_apply_edits(self):
        seq = "ACGGTGCAGTC"
        edits = [(1, "+G"), (4, "C**"), (8, "-")]
        expected = "AGCGGCGCATC"
        self.assertEqual(aiedit.core.apply_edits(seq, edits), expected)
