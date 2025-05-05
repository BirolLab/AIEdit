import datetime
import math
import os
import pathlib
import sys

from aiedit import __version__, core


def _get_ref_alt(seq: str, edit: core.Edit):
    ref, alt = ".", "."
    if edit.status == core.EditStatus.MODEL_FAIL:
        return ref, alt
    if edit.type == core.EditType.SUBSTITUTE:
        ref = seq[edit.position : edit.position + len(edit.edited)]
        alt = edit.edited
    elif edit.type == core.EditType.INSERT:
        ref = seq[edit.position - 1]
        alt = seq[edit.position - 1] + edit.edited
    elif edit.type == core.EditType.DELETE:
        ref = seq[edit.position - 1 : edit.position + len(edit.edited)]
        alt = seq[edit.position - 1]
    return ref, alt


def _get_status_filter(status: core.EditStatus, min_qual: int):
    if status == core.EditStatus.PASS:
        return "PASS"
    elif status == core.EditStatus.MODEL_FAIL:
        return "fail"
    elif status == core.EditStatus.LOW_KMER_SCORE:
        return f"qual{min_qual}"
    return "."


class VCFWriter:

    def __init__(self, assembly_path: str, min_score: float):
        self._min_qual = int(-10 * math.log10(min_score))
        self._file_lines = [
            "##fileformat=VCFv4.5",
            f"##fileDate={datetime.date.today().strftime('%Y%m%d')}",
            f"##source=AIEditV{__version__}",
            f"##assembly={pathlib.Path(os.path.abspath(assembly_path)).as_uri()}",
            "",  # contigs to be added at runtime
            "##INFO=<ID=RL,Number=1,Type=Integer,Description=Edit region length in bp>",
            f"##FILTER=<ID=qual{self._min_qual},Description=K-mer Phred score less than {self._min_qual} after edits>",
            "##FILTER=<ID=fail,Description=Model did not detect any edits>",
            "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO",
        ]

    def add(self, seq_id: str, comment: str, seq: str, edits: core.EditList):
        head = f"##contig=<ID={seq_id},length={len(seq)},description={comment or 'NA'}>"
        self._file_lines[4] += os.linesep if self._file_lines[4] else ""
        self._file_lines[4] += head
        for edit in edits:
            kmer_score = edit.score if not math.isnan(edit.score) else 0
            row = (
                seq_id,
                edit.position,
                len(self._file_lines) - 9,
                *_get_ref_alt(seq, edit),
                int(-10 * math.log10(1 - kmer_score + sys.float_info.epsilon)),
                _get_status_filter(edit.status, self._min_qual),
                ".",
            )
            self._file_lines.append("\t".join(map(str, row)))

    def write(self, path: str):
        with open(path, "w") as fp:
            fp.write(os.linesep.join(self._file_lines))
