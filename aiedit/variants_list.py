import dataclasses
import datetime
import math
import os
import pathlib
import sys

from aiedit import __version__


@dataclasses.dataclass
class Contig:
    name: str
    comment: str
    length: int
    md5_hash: int = 0

    def vcf_header(self):
        return f"##contig=<ID={self.name},length={self.length},md5={self.md5_hash},description={self.comment or 'NA'}>"


@dataclasses.dataclass
class Variant:
    index: int
    seq_id: str
    position: int
    ref: str
    alt: str
    kmer_score: int
    region_length: int
    status: str

    @staticmethod
    def from_interface_output(output: tuple, index: int, seq_id: str, seq: str):
        position, region_length, edit, score, passed = output
        if math.isnan(score):
            score = 0
        status = "PASS" if passed else "ks"
        edit = edit.rstrip("*")
        if len(edit) == 0:
            ref, alt = ".", "."
            status = "ml"
        elif edit[0] == "-":
            ref = "."
            alt = seq[position : position + edit.count("-")]
        elif edit[0] == "+":
            ref = edit[1::2] + seq[position]
            alt = seq[position]
        else:
            ref = (seq[position + i] if b == "*" else b for i, b in enumerate(edit))
            ref = "".join(ref)
            alt = seq[position : position + len(edit)]
        return Variant(
            index=index,
            seq_id=seq_id,
            position=position + 1,
            ref=ref,
            alt=alt,
            kmer_score=int(-10 * math.log10(1 - score + sys.float_info.epsilon)),
            region_length=region_length,
            status=status,
        )

    def vcf_row(self):
        row = [
            self.seq_id,
            self.position,
            self.index,
            self.ref,
            self.alt,
            self.kmer_score,
            self.status,
            self.region_length,
        ]
        return "\t".join(map(str, row))


class VariantsList:

    def __init__(self):
        self._contigs: list[Contig] = []
        self._variants: list[Variant] = []

    def add(
        self,
        edits: list[tuple[int, str]],
        seq: str,
        seq_id: str,
        seq_comment: str,
        seq_length: int,
    ):
        self._contigs.append(Contig(seq_id, seq_comment, seq_length))
        for edit in edits:
            index = len(self._variants)
            variant = Variant.from_interface_output(edit, index, seq_id, seq)
            if variant.ref != variant.alt:
                self._variants.append(variant)

    def save_vcf(self, path: str, assembly_path: str):
        header = [
            "##fileformat=VCFv4.5",
            f"##fileDate={datetime.date.today().strftime('%Y%m%d')}",
            f"##source=AIEditV{__version__}",
            f"##assembly={pathlib.Path(os.path.abspath(assembly_path)).as_uri()}",
        ]
        header += [contig.vcf_header() for contig in self._contigs]
        header += [
            "##INFO=<ID=RL,Number=1,Type=Integer,Description=Edit region length in bp>",
            "##FILTER=<ID=ks,Description=K-mer score did not increase>",
            "##FILTER=<ID=ml,Description=Model did not detect any edits>",
            "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO",
        ]
        with open(path, "w") as file:
            file.write(os.linesep.join(header))
            file.write(os.linesep)
            file.write(os.linesep.join(variant.vcf_row() for variant in self._variants))
