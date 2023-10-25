import dataclasses
import numpy as np
from tqdm import tqdm
from signature import get_signature


@dataclasses.dataclass
class Dataset:
    x_train: list[np.array]
    y_train: list[np.array]
    x_test: list[np.array]
    y_test: list[np.array]
    patterns: list[str]


def get_pattern_strings(pattern_length: int) -> list[str]:
    pattern_strings = []
    for i in range(2 ** (pattern_length - 1), 2**pattern_length):
        p_str = np.base_repr(i, 2).zfill(pattern_length).replace("1", "M")
        pattern_strings.append(p_str)
    for i in range(1, pattern_length + 1):
        pattern_strings.append("I" * i + "0" * (pattern_length - i))
        pattern_strings.append("D" * i + "0" * (pattern_length - i))
    return pattern_strings


def get_pattern_tensor(pattern_string: str) -> np.array:
    pattern = np.zeros(2 ** len(pattern_string))
    i1 = 0
    for i in range(len(pattern_string)):
        if pattern_string[i] == "M":
            i1 += 2**i
    pattern[i1] = 1.0
    return pattern


def prepare_data(
    seeds: list[str], pattern_length: int, samples_per_class: int, fpr: float
) -> Dataset:
    x_train, y_train, x_test, y_test = [], [], [], []
    patterns = []
    for p in tqdm(
        get_pattern_strings(pattern_length), desc="Generating data", unit="patterns"
    ):
        for i in range(max(2, int(samples_per_class * 1.2))):
            signature = get_signature(seeds, p, fpr if i > 0 else 0)
            if i < samples_per_class:
                x_train.append(signature)
                y_train.append(get_pattern_tensor(p))
            else:
                x_test.append(signature)
                y_test.append(get_pattern_tensor(p))
            patterns.append(p)
    return Dataset(x_train, y_train, x_test, y_test, patterns)
