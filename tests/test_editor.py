import unittest

import aiedit


class TestEditor(unittest.TestCase):

    def setUp(self):
        self.seq = "GTCGCTAGACTGATAG"
        self.editor = aiedit.core.Editor(self.seq, 0, len(self.seq))

    def test_initialization(self):
        self.assertEqual(self.editor.get_size(), len(self.seq))
        self.assertEqual(self.editor.get_num_remaining(), len(self.seq))
        self.assertEqual(self.editor.get_current(), self.seq[0])

    def test_skip(self):
        self.editor.skip()
        self.assertEqual(self.editor.get_num_remaining(), len(self.seq) - 1)
        self.assertEqual(self.editor.get_current(), self.seq[1])
        self.assertEqual(self.editor.get_size(), len(self.seq))

    def test_substitution(self):
        self.editor.substitute("C")
        self.assertEqual(self.editor.get_current(), "C")
        self.assertEqual("".join(c for c in self.editor), "CTCGCTAGACTGATAG")
        self.assertEqual(self.editor.get_num_remaining(), len(self.seq))
        self.assertEqual(self.editor.get_size(), len(self.seq))
        self.editor.substitute("T")
        self.assertEqual(self.editor.get_current(), "T")
        self.assertEqual("".join(c for c in self.editor), "TTCGCTAGACTGATAG")

    def test_insertion(self):
        self.editor.insert("C")
        self.assertEqual("".join(c for c in self.editor), "CGTCGCTAGACTGATAG")
        self.assertEqual(self.editor.get_num_remaining(), len(self.seq))
        self.assertEqual(self.editor.get_size(), len(self.seq) + 1)

    def test_deletion(self):
        self.editor.delete_base()
        self.assertEqual("".join(c for c in self.editor), "TCGCTAGACTGATAG")
        self.assertEqual(self.editor.get_num_remaining(), len(self.seq) - 1)
        self.assertEqual(self.editor.get_size(), len(self.seq) - 1)

    def test_multiple_edits(self):
        self.editor.substitute("T")
        self.editor.skip()
        self.editor.delete_base()
        self.editor.skip()
        self.editor.insert("A")
        self.assertEqual("".join(c for c in self.editor), "TCAGCTAGACTGATAG")
        self.assertEqual(self.editor.get_size(), len(self.seq))

    def test_full_deletion(self):
        for _ in range(len(self.seq)):
            self.editor.delete_base()
        self.assertEqual(self.editor.get_size(), 0)
        self.assertRaises(RuntimeError, self.editor.skip)
        self.assertRaises(RuntimeError, self.editor.delete_base)
        self.assertRaises(RuntimeError, self.editor.substitute, "G")
        self.editor.insert("A")
        self.editor.insert("C")
        self.editor.insert("G")
        self.editor.insert("T")
        self.assertEqual("".join(c for c in self.editor), "ACGT")
