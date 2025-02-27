import unittest

import aiedit


class TestEditBindings(unittest.TestCase):

    def test_substitution(self):
        edit = aiedit.Edit.substitution(0, "A")
        self.assertEqual(edit.position, 0)
        self.assertEqual(edit.type, aiedit.EditType.SUBSTITUTE)
        self.assertEqual(edit.new_base, "A")

    def test_insertion(self):
        edit = aiedit.Edit.insertion(0, "C")
        self.assertEqual(edit.position, 0)
        self.assertEqual(edit.type, aiedit.EditType.INSERT)
        self.assertEqual(edit.new_base, "C")

    def test_deletion(self):
        edit = aiedit.Edit.deletion(0)
        self.assertEqual(edit.position, 0)
        self.assertEqual(edit.type, aiedit.EditType.DELETE)
