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
    original: str
    edited: str
    kmer_score: int
    region_length: int
    status: str

    @staticmethod
    def from_interface(
        output: tuple[int, int, str, float],
        index: int,
        seq_id: str,
        seq: str,
        min_score: float,
    ):
        position, region_length, edit, score = output
        if math.isnan(score):
            score = 0
        if score >= min_score:
            status = "PASS"
        else:
            status = f"ks{int(-10 * math.log10(min_score))}"
        edit = edit.rstrip("*")
        if len(edit) == 0:
            edited, original = seq[position], seq[position]
            status = "ml"
        elif edit[0] == "-":
            num_deleted = edit.count("-")
            edited = seq[position - 1]
            original = seq[position - 1 : position + num_deleted]
        elif edit[0] == "+":
            edited = seq[position - 1] + edit[1::2]
            original = seq[position - 1]
        else:
            edited = (seq[position + i] if b == "*" else b for i, b in enumerate(edit))
            edited = "".join(edited)
            original = seq[position : position + len(edit)]
        return Variant(
            index=index,
            seq_id=seq_id,
            position=position + 1,
            original=original,
            edited=edited,
            kmer_score=int(-10 * math.log10(1 - score + sys.float_info.epsilon)),
            region_length=region_length,
            status=status,
        )

    def vcf_row(self):
        row = [
            self.seq_id,
            self.position,
            self.index,
            self.original,
            self.edited,
            self.kmer_score,
            self.status,
            self.region_length,
        ]
        return "\t".join(map(str, row))


class VariantsList:

    def __init__(self, score_threshold: float):
        self._min_score = score_threshold
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
            var = Variant.from_interface(edit, index, seq_id, seq, self._min_score)
            if var.edited != var.original:
                self._variants.append(var)

    def save_vcf(self, path: str, assembly_path: str):
        header = [
            "##fileformat=VCFv4.5",
            f"##fileDate={datetime.date.today().strftime('%Y%m%d')}",
            f"##source=AIEditV{__version__}",
            f"##assembly={pathlib.Path(os.path.abspath(assembly_path)).as_uri()}",
        ]
        header += [contig.vcf_header() for contig in self._contigs]
        min_qual = int(-10 * math.log10(self._min_score))
        header += [
            "##INFO=<ID=RL,Number=1,Type=Integer,Description=Edit region length in bp>",
            f"##FILTER=<ID=qual{min_qual},Description=K-mer Phred score less than {min_qual} after edits>",
            "##FILTER=<ID=ml,Description=Model did not detect any edits>",
            "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO",
        ]
        with open(path, "w") as file:
            file.write(os.linesep.join(header))
            file.write(os.linesep)
            file.write(os.linesep.join(variant.vcf_row() for variant in self._variants))
