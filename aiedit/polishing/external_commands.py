import importlib
import os
import shutil
import subprocess
import sys
import tempfile
import time

import ntstat.filter

from aiedit import core


def run_ntcard(
    reads_paths: list[str], kmer_size: int, num_threads: int, out_prefix: str
):
    hist_path = f"{out_prefix}-ntcard_k{kmer_size}.hist"
    if os.path.exists(hist_path):
        print(f"ntCard output detected: {hist_path}")
        return hist_path
    print("Running ntCard for k-mer count histogram...")
    reads = " ".join(reads_paths)
    command = f"ntcard -p {out_prefix}-ntcard -k {kmer_size} -t {num_threads} {reads}"
    print(f"Command: {command}")
    start_time = time.perf_counter()
    result = subprocess.run(
        command.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
    )
    end_time = time.perf_counter()
    if result.returncode != 0:
        print("ntCard failed with code", result.returncode, file=sys.stderr)
        print("stdout:", os.linesep, result.stdout, file=sys.stderr)
        print("stderr:", os.linesep, result.stderr, file=sys.stderr)
        sys.exit(1)
    print(f"Histogram saved to {hist_path} ({end_time - start_time:.1f}s)")
    print()
    return hist_path


def run_nstat_kmer(
    reads_paths: list[str],
    hist_path: str,
    kmer_size: int,
    num_threads: int,
    out_prefix: str,
) -> str:
    bf_path = f"{out_prefix}-kmers.bf"
    if os.path.exists(bf_path):
        print(f"ntStat output detected: {bf_path}")
        return bf_path
    print("Running ntStat for k-mers Bloom filter...")
    reads = " ".join(reads_paths)
    command = f"filter -k {kmer_size} -cmin 0 -t {num_threads} -f {hist_path} -o {bf_path} {reads}"
    print(f"Command: ntstat {command}")
    print("Output:")
    print("-" * shutil.get_terminal_size().columns)
    start_time = time.perf_counter()
    return_code = ntstat.filter.run(command.split())
    end_time = time.perf_counter()
    print("-" * shutil.get_terminal_size().columns)
    if return_code != 0:
        print("ntStat failed with code", return_code, file=sys.stderr)
        sys.exit(1)
    print(f"K-mers BF saved to {bf_path} ({end_time - start_time:.1f}s)")
    print()
    return bf_path


def run_ntstat_seeds(
    reads_paths: list[str],
    hist_path: str,
    model_path: str,
    kmer_size: int,
    num_threads: int,
    out_prefix: str,
):
    bf_path = f"{out_prefix}-seeds.bf"
    if os.path.exists(bf_path):
        print(f"ntStat output detected: {bf_path}")
        return bf_path
    print("Running ntStat for k-mers Bloom filter...")
    torch = importlib.import_module("torch")
    model = torch.jit.load(model_path)
    seeds = core.SeedGenerator(100, 100, 0.5).generate(
        model.num_seeds, kmer_size, model.max_mismatches, model.max_indels
    )
    seeds = sorted(seeds, key=lambda seed: seed.count("0"))
    seeds_file = tempfile.NamedTemporaryFile(prefix="aiedit-seeds-", suffix=".txt")
    seeds_file.write(os.linesep.join(seeds).encode())
    seeds_file.seek(0)
    reads = " ".join(reads_paths)
    command = f"filter -s {seeds_file.name} -cmin 0 -t {num_threads} -f {hist_path} -o {bf_path} {reads}"
    print(f"Command: ntstat {command}")
    print("Output:")
    print("-" * shutil.get_terminal_size().columns)
    start_time = time.perf_counter()
    return_code = ntstat.filter.run(command.split())
    end_time = time.perf_counter()
    print("-" * shutil.get_terminal_size().columns)
    seeds_file.close()
    if return_code != 0:
        print("ntStat failed with code", return_code, file=sys.stderr)
        sys.exit(1)
    print(f"Seeds BF saved to {bf_path} ({end_time - start_time:.1f}s)")
    print()
    return bf_path


def run_ntedit(bf_path: str, edited_path: str, num_threads: int, out_prefix: str):
    print("Running ntEdit...")
    command = f"ntedit -r {bf_path} -f {edited_path} -t {num_threads} -b {out_prefix}"
    print(f"Command: {command}")
    start_time = time.perf_counter()
    result = subprocess.run(
        command.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
    )
    end_time = time.perf_counter()
    if result.returncode != 0:
        print("ntEdit failed with code", result.returncode, file=sys.stderr)
        print("stdout:", os.linesep, result.stdout, file=sys.stderr)
        print("stderr:", os.linesep, result.stderr, file=sys.stderr)
        sys.exit(1)
    results_path = f"{out_prefix}_edited.fa"
    print(f"ntEdit results saved to {results_path} ({end_time - start_time:.1f}s)")
    return results_path
