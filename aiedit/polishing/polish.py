import os
import pathlib
import sys
import time

import btllib

from aiedit import core
from aiedit.polishing import external_commands
from aiedit.polishing.vcf_writer import VCFWriter


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
    print(f"- Type: {'COUNTING ' if is_cbf else ''}BLOOM FILTER")
    print(f"- Size (bytes): {kmer_model.get_size():,}")
    print(f"- Number of seeds: {len(kmer_model.get_seeds())}")
    print(f"- K-mer size: {kmer_model.get_kmer_size()}")
    print()
    return kmer_model


def load_polisher(
    model_path: str, kmer_model: core.KmerModel, num_threads: int, hit_prob: float
) -> core.Polisher:
    print("Loading edit pattern model... ", end="", flush=True)
    start_time = time.perf_counter()
    polisher = core.Polisher(model_path, kmer_model, num_threads, hit_prob)
    end_time = time.perf_counter()
    print(f"DONE ({end_time - start_time:.1f}s)")
    print(f"- Maximum consecutive substitutions: {polisher.get_max_mismatches()}")
    print(f"- Maximum insertion/deletion size: {polisher.get_max_indels()}")
    print()
    return polisher


def main(args):
    os.makedirs(args.out_path, exist_ok=True)
    out_prefix = os.path.join(args.out_path, pathlib.Path(args.input_file).stem)

    if str.isdigit(args.kmers) and not args.reads:
        print("ERROR: must pass reads (-r) if -k is k-mer size", file=sys.stderr)
        sys.exit(1)

    if str.isdigit(args.kmers):
        kmer_size = int(args.kmers)
        hist_path = external_commands.run_ntcard(
            args.reads, kmer_size, args.threads, out_prefix
        )
        args.kmers = external_commands.run_nstat_kmer(
            args.reads, hist_path, kmer_size, args.threads, out_prefix
        )
        args.seeds = external_commands.run_ntstat_seeds(
            args.reads, hist_path, args.model, kmer_size, args.threads, out_prefix
        )

    kmer_model = load_kmer_model(args.kmers, args.seeds, args.hist_model)
    polisher = load_polisher(args.model, kmer_model, args.threads, args.hit_prob)

    out_fasta_path = f"{out_prefix}-aiedit-polished.fa"
    out_vcf_path = f"{out_prefix}-aiedit-changes.vcf"

    seq_reader = btllib.SeqReader(args.input_file, btllib.SeqReaderFlag.LONG_MODE)
    seq_writer = btllib.SeqWriter(out_fasta_path)
    vcf_writer = VCFWriter(args.input_file, args.hit_prob)
    for record in seq_reader:
        print(f"[{record.id}] Processing started ({len(record.seq):,}bp)")
        start_time = time.perf_counter()
        edits = polisher.polish(record.seq)
        end_time = time.perf_counter()
        elapsed = end_time - start_time
        print(f"[{record.id}] Found {len(edits)} edits in {elapsed:.1f}s")
        vcf_writer.add(record.id, record.comment, record.seq, edits)
        edited = edits.apply(record.seq)
        num_passed = edits.get_num_passed()
        print(f"[{record.id}] Applied {num_passed} passed edits ({len(edited):,}bp)")
        seq_writer.write(record.id, record.comment, edited)
        print(f"[{record.id}] Edited sequence saved to {out_fasta_path}")

    vcf_writer.write(out_vcf_path)
    print(f"Edits saved to {out_vcf_path}")
    print()

    if args.ntedit:
        external_commands.run_ntedit(
            args.kmers, out_fasta_path, args.threads, out_prefix
        )
