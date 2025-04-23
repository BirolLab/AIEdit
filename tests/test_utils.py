import unittest

import aiedit


class TestUtils(unittest.TestCase):

    def test_apply_edits(self):
        seq = "ACGGTGCAGTC"
        edits = [
            (1, 1, "+G", 1, True),
            (4, 1, "C**", 1, True),
            (8, 1, "-", 1, True),
            (9, 1, "T", 0, False),
        ]
        expected = "AGCGGCGCATC"
        self.assertEqual(aiedit.core.apply_edits(seq, edits), expected)
