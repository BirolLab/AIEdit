import unittest

from aiedit.variants_list import Variant


class TestVariants(unittest.TestCase):

    def test_substitution(self):
        ref, alt = "CTA", "GTC"
        variant = Variant.from_interface((0, 3, "C*A", 1), 0, "", alt, 0.5)
        self.assertEqual(variant.edited, ref)
        self.assertEqual(variant.original, alt)

    def test_insertion(self):
        variant = Variant.from_interface((1, 1, "-", 1), 0, "", "CTA", 0.5)
        self.assertEqual(variant.edited, "C")
        self.assertEqual(variant.original, "CT")

    def test_deletion(self):
        variant = Variant.from_interface((1, 1, "+G", 1), 0, "", "CTA", 0.5)
        self.assertEqual(variant.edited, "CG")
        self.assertEqual(variant.original, "C")
