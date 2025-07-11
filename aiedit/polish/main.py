import math
import os
import pathlib
import re
import sys
import time

import btllib

from aiedit import core
from aiedit.polish import external_commands
from aiedit.polish.vcf_writer import VCFWriter


def load_kmer_model(
    kmers_bf: str, seeds_bf: str, hist_model: str | None
) -> core.KmerModel:
    print("Loading Bloom filters... ", end="", flush=True)
    start_time = time.perf_counter()
    if btllib.KmerBloomFilter.is_bloom_file(kmers_bf):
        kmer_model = core.BFKmerModel(seeds_bf, kmers_bf)
    elif btllib.KmerCountingBloomFilter8.is_bloom_file(kmers_bf) and hist_model:
        kmer_model = core.CBFKmerModel(seeds_bf, kmers_bf, hist_model)
    elif btllib.KmerCountingBloomFilter8.is_bloom_file(kmers_bf):
        kmer_model = core.CBFKmerModel(seeds_bf, kmers_bf)
    else:
        print()
        print(
            f"ERROR: {kmers_bf} is not a valid Bloom filter "
            "(btllib::KmerBloomFilter or btllib::KmerCountingBloomFilter8)",
            file=sys.stderr,
        )
        sys.exit(1)
    end_time = time.perf_counter()
    is_cbf = isinstance(kmer_model, core.CBFKmerModel)
    print(f"DONE ({end_time - start_time:.1f}s)")
    print(f"- K-mer size: {kmer_model.get_kmer_size()}")
    print(f"- K-mer counts: {'un' if not is_cbf else ''}available")
    print(f"- Number of seeds: {len(kmer_model.get_seeds())}")
    print(f"- Size (bytes): {kmer_model.get_size():,}")
    print(f"- False positive rate (k-mers): {kmer_model.get_kmers_fpr():.4f}")
    print(f"- False positive rate (seeds): {kmer_model.get_seeds_fpr():.4f}")
    print()
    return kmer_model


def load_polisher(
    model_path: str,
    kmer_model: core.KmerModel,
    num_threads: int,
    hit_prob: float,
    num_tries: int,
) -> core.Polisher:
    print("Loading edit pattern model... ", end="", flush=True)
    start_time = time.perf_counter()
    polisher = core.Polisher(model_path, kmer_model, num_threads, hit_prob, num_tries)
    end_time = time.perf_counter()
    print(f"DONE ({end_time - start_time:.1f}s)")
    print(f"- Maximum consecutive substitutions: {polisher.get_max_mismatches()}")
    print(f"- Maximum insertion/deletion size: {polisher.get_max_indels()}")
    print(f"- Number of top patterns: {num_tries}")
    print()
    return polisher


def get_common_prefix(file_names: list[str]) -> str:
    common_index = 0
    min_length = min(len(name) for name in file_names)
    while common_index < min_length:
        if len(set(name[common_index] for name in file_names)) != 1:
            break
        common_index += 1
    return file_names[0][:common_index]


def remove_trailing_symbols(file_name: str) -> str:
    return re.sub(r"[^a-zA-Z0-9]+$", "", file_name)


def main(args):
    os.makedirs(args.out_path, exist_ok=True)
    out_prefix = os.path.join(args.out_path, pathlib.Path(args.assembly).stem)

    if str.isdigit(args.kmers) and not args.reads:
        print("ERROR: must pass reads (-r) if -k is k-mer size", file=sys.stderr)
        sys.exit(1)

    if str.isdigit(args.kmers):
        reads_common = remove_trailing_symbols(get_common_prefix(args.reads)) or "reads"
        reads_out_prefix = os.path.join(args.out_path, pathlib.Path(reads_common).stem)
        kmer_size = int(args.kmers)
        hist_path = external_commands.run_ntcard(
            args.reads, kmer_size, args.threads, reads_out_prefix
        )
        args.kmers = external_commands.run_nstat_kmer(
            args.reads, hist_path, kmer_size, args.threads, reads_out_prefix
        )
        args.seeds = external_commands.run_ntstat_seeds(
            args.reads, hist_path, args.model, kmer_size, args.threads, reads_out_prefix
        )
        print()

    kmer_model = load_kmer_model(args.kmers, args.seeds, args.hist_model)
    polisher = load_polisher(
        args.model,
        kmer_model,
        args.threads,
        args.hit_prob,
        args.num_tries,
    )

    out_fasta_path = f"{out_prefix}_edited.fa"
    out_vcf_path = f"{out_prefix}_aiedit_variants.vcf"

    seq_reader = btllib.SeqReader(args.assembly, btllib.SeqReaderFlag.LONG_MODE)
    seq_writer = btllib.SeqWriter(out_fasta_path)
    vcf_writer = VCFWriter(args.assembly, args.hit_prob)
    for record in seq_reader:
        print("Processing", record.id)
        start_time = time.perf_counter()
        edits = polisher.polish(record.seq)
        end_time = time.perf_counter()
        elapsed = end_time - start_time
        print(f"- Found {len(edits)} edits ({elapsed:.1f}s)")
        vcf_writer.add(record.id, record.comment, record.seq, edits)
        edited = edits.apply(record.seq)
        num_passed = edits.get_num_passed()
        seq_writer.write(record.id, record.comment, edited)
        print(f"- Applied {num_passed} passed edits")
        print(f"- Sequence length: {len(record.seq):,}bp -> {len(edited):,}bp")
        print()

    vcf_writer.write(out_vcf_path)
    print(f"Edits saved to {out_vcf_path}")

    if args.ntedit:
        print()
        out_fasta_path = external_commands.run_ntedit(
            args.kmers, out_fasta_path, args.threads, out_prefix
        )

    print()
    print(f"Edited sequences saved to {out_fasta_path}")
