import random
import tempfile
import typing

import btllib
import torch

from aiedit import core


def buffer2d_to_tensor(buffer: core.Buffer2D) -> torch.Tensor:
    shape = (buffer.num_rows, buffer.num_cols)
    return torch.frombuffer(buffer, dtype=torch.float).view(*shape)


def encode_seeds(seeds: list[str]) -> torch.FloatTensor:
    x_seeds = torch.tensor([list(map(int, seed)) for seed in seeds])
    return x_seeds.permute(1, 0).float()


def _make_kmer_model(seq: str, seeds: list[str]) -> core.BFKmerModel:
    bf_seeds = btllib.SeedBloomFilter(1024 * 1024, len(seeds[0]), seeds, 7)
    bf_seeds.insert(seq)
    bf_kmers = btllib.KmerBloomFilter(1024 * 1024, len(seeds[0]), 7)
    bf_kmers.insert(seq)
    with tempfile.NamedTemporaryFile() as fp1, tempfile.NamedTemporaryFile() as fp2:
        bf_seeds.save(fp1.name)
        bf_kmers.save(fp2.name)
        kmer_model = core.BFKmerModel(fp1.name, fp2.name)
    return kmer_model


def _get_mismatch_signature(
    seq: str, pattern: str, max_indels: int, kmer_model: core.BFKmerModel
) -> torch.FloatTensor:
    edited = list(seq)
    for pos, _ in filter(lambda x: x[1] == "1", enumerate(pattern)):
        seq_pos = pos + kmer_model.get_kmer_size()
        bases = ["A", "C", "G", "T"]
        bases.remove(edited[seq_pos])
        edited[seq_pos] = random.choice(bases)
    end = pattern.rindex("1") + kmer_model.get_kmer_size() + 1
    interface = core.ModelInterface("".join(edited), 1, end, max_indels, kmer_model)
    return buffer2d_to_tensor(interface.get_signature())


def _get_insertion_sample(
    seq: str, num_ins: int, max_indels: int, kmer_model: core.BFKmerModel
) -> torch.FloatTensor:
    k = kmer_model.get_kmer_size()
    insertion = "".join(random.choices("ACGT", k=num_ins))
    edited = seq[:k] + insertion + seq[k:]
    interface = core.ModelInterface(edited, 1, num_ins + k, max_indels, kmer_model)
    return buffer2d_to_tensor(interface.get_signature())


def _get_deletion_sample(
    seq: str, num_del: int, max_indels: int, kmer_model: core.BFKmerModel
) -> torch.FloatTensor:
    k = kmer_model.get_kmer_size()
    edited = seq[:k] + seq[k + num_del :]
    interface = core.ModelInterface(edited, 1, k, max_indels, kmer_model)
    return buffer2d_to_tensor(interface.get_signature())


def generate_dataset(
    seeds: list[str], max_mismatches: int, max_indels: int
) -> typing.Generator[tuple[tuple[typing.Optional[torch.FloatTensor]]], None, None]:
    seq = "".join(random.choices("ACGT", k=len(seeds[0]) * 2 + max_mismatches))
    kmer_model = _make_kmer_model(seq, seeds)
    x_seeds = encode_seeds(seeds)
    for i in range(0, 2 ** (max_mismatches - 1)):
        pattern = "1" + format(i, f"0{max_mismatches - 1}b")
        x_sig = _get_mismatch_signature(seq, pattern, max_indels, kmer_model)
        y_mis = torch.tensor(list(map(int, pattern))).unsqueeze(0).float()
        yield (x_seeds, x_sig), (torch.zeros(1, 1), y_mis, None)
    for n in range(1, max_indels + 1):
        x_sig = _get_insertion_sample(seq, n, max_indels, kmer_model)
        y_ins = torch.zeros(1, max_indels * 2)
        y_ins[0, n - 1] = 1.0
        yield (x_seeds, x_sig), (torch.ones(1, 1), None, y_ins)
        x_sig = _get_deletion_sample(seq, n, max_indels, kmer_model)
        y_del = torch.zeros(1, max_indels * 2)
        y_del[0, n + max_indels - 1] = 1.0
        yield (x_seeds, x_sig), (torch.ones(1, 1), None, y_del)
