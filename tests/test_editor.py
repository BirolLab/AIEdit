import unittest

import aiedit


class TestEditor(unittest.TestCase):

    def setUp(self):
        self.seq = "GTCGCTAGACTGATAG"
        self.editor = aiedit.core.Editor(self.seq, 0, len(self.seq))

    def test_size(self):
        self.assertEqual(self.editor.size, len(self.seq))

    def test_substitution(self):
        self.editor.substitute("C")
        self.assertEqual("".join(c for c in self.editor), "CTCGCTAGACTGATAG")
        self.assertEqual(self.editor.size, len(self.seq))

    def test_insertion(self):
        self.editor.insert("C")
        self.assertEqual("".join(c for c in self.editor), "CGTCGCTAGACTGATAG")
        self.assertEqual(self.editor.size, len(self.seq) + 1)

    def test_deletion(self):
        self.editor.delete_base()
        self.assertEqual("".join(c for c in self.editor), "TCGCTAGACTGATAG")
        self.assertEqual(self.editor.size, len(self.seq) - 1)

    def test_multiple_edits(self):
        self.editor.substitute("T")
        self.editor.skip()
        self.editor.delete_base()
        self.editor.skip()
        self.editor.insert("A")
        self.assertEqual("".join(c for c in self.editor), "TTGACTAGACTGATAG")
        self.assertEqual(self.editor.position, 4)
        self.assertEqual(self.editor.size, len(self.seq))

    def test_full_deletion(self):
        for _ in range(len(self.seq)):
            self.editor.delete_base()
        self.assertEqual(self.editor.size, 0)
        self.assertRaises(RuntimeError, self.editor.skip)
        self.assertRaises(RuntimeError, self.editor.delete_base)
        self.assertRaises(RuntimeError, self.editor.substitute, "G")
        self.editor.insert("A")
        self.editor.insert("C")
        self.editor.insert("G")
        self.editor.insert("T")
        self.assertEqual("".join(c for c in self.editor), "ACGT")
