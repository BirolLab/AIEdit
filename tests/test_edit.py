import unittest

import aiedit


class TestEditBindings(unittest.TestCase):

    def test_substitution(self):
        edit = aiedit.core.Edit.substitution(0, "A")
        self.assertEqual(edit.position, 0)
        self.assertEqual(edit.type, aiedit.core.EditType.SUBSTITUTE)
        self.assertEqual(edit.new_base, "A")

    def test_insertion(self):
        edit = aiedit.core.Edit.insertion(0, "C")
        self.assertEqual(edit.position, 0)
        self.assertEqual(edit.type, aiedit.core.EditType.INSERT)
        self.assertEqual(edit.new_base, "C")

    def test_deletion(self):
        edit = aiedit.core.Edit.deletion(0)
        self.assertEqual(edit.position, 0)
        self.assertEqual(edit.type, aiedit.core.EditType.DELETE)
