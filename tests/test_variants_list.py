import unittest

from aiedit.variants_list import Variant


class TestVariants(unittest.TestCase):

    def test_substitution(self):
        ref, alt = "CTA", "GTC"
        variant = Variant.from_interface_output((0, 3, "C*A", 1, True), 0, "", alt)
        self.assertEqual(variant.ref, ref)
        self.assertEqual(variant.alt, alt)

    def test_insertion(self):
        variant = Variant.from_interface_output((1, 1, "-", 1, True), 0, "", "CTA")
        self.assertEqual(variant.ref, ".")
        self.assertEqual(variant.alt, "T")

    def test_deletion(self):
        variant = Variant.from_interface_output((1, 1, "+G", 1, True), 0, "", "CTA")
        self.assertEqual(variant.ref, "GT")
        self.assertEqual(variant.alt, "T")
