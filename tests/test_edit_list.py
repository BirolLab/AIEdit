import unittest

import aiedit


class TestVariants(unittest.TestCase):

    def test_substitution(self):
        ref, alt = "CTA", "GTA"
        edit = aiedit.core.Edit()
        edit.position = 1
        edit.type = aiedit.core.EditType.SUBSTITUTE
        edit.edited = "G"
        edit_list = aiedit.core.EditList()
        edit_list.add(edit)
        self.assertEqual(alt, edit_list.apply(ref))

    def test_insertion(self):
        ref, alt = "CA", "CTA"
        edit = aiedit.core.Edit()
        edit.position = 1
        edit.type = aiedit.core.EditType.INSERT
        edit.edited = "T"
        edit_list = aiedit.core.EditList()
        edit_list.add(edit)
        self.assertEqual(alt, edit_list.apply(ref))

    def test_deletion(self):
        ref, alt = "CTA", "CA"
        edit = aiedit.core.Edit()
        edit.position = 1
        edit.type = aiedit.core.EditType.DELETE
        edit.edited = "-"
        edit_list = aiedit.core.EditList()
        edit_list.add(edit)
        self.assertEqual(alt, edit_list.apply(ref))
