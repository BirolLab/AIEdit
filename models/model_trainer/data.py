import numpy as np
import numpy.typing as np_types
from tqdm import tqdm
from signature import get_signature


def _pattern_to_array(pattern: str) -> np_types.ArrayLike:
    return np.array([1.0 if b == "M" else 0.0 for b in pattern])


def _get_pattern_strings(pattern_length: int) -> list[str]:
    pattern_strings = []
    for i in range(2 ** (pattern_length - 1), 2**pattern_length):
        p_str = np.base_repr(i, 2).zfill(pattern_length).replace("1", "M")
        pattern_strings.append(p_str)
    for i in range(1, pattern_length + 1):
        pattern_strings.append("I" * i + "0" * (pattern_length - i))
        pattern_strings.append("D" * i + "0" * (pattern_length - i))
    return pattern_strings


class Dataset:
    def __init__(
        self, seeds: list[str], pattern_length: int, class_size: int, fpr: float
    ) -> None:
        self.__seeds = seeds
        self.__pattern_length = pattern_length
        self.__class_size = class_size
        self.__fpr = fpr
        self.__x_train = []
        self.__y_train = []
        self.__x_test = []
        self.__y_test = []
        self.__prepare_data()

    @property
    def x_train(self):
        return np.array(self.__x_train)

    @property
    def y_train(self):
        return np.array(self.__y_train)

    @property
    def x_test(self):
        return np.array(self.__x_test)

    @property
    def y_test(self):
        return np.array(self.__y_test)
    
    def print_details(self):
        print(f"Training data size: {len(self.__x_train)}")
        print(f"Testing data size: {len(self.__x_test)}")

    def __populate_class(self, pattern: str):
        for i in range(max(2, int(self.__class_size * 1.2))):
            signature = get_signature(self.__seeds, pattern, self.__fpr if i > 0 else 0)
            if i < self.__class_size:
                self.__x_train.append(signature)
                self.__y_train.append(_pattern_to_array(pattern))
            else:
                self.__x_test.append(signature)
                self.__y_test.append(_pattern_to_array(pattern))

    def __prepare_data(self):
        patterns = _get_pattern_strings(self.__pattern_length)
        for p in tqdm(patterns, desc="Generating data", unit="patterns"):
            self.__populate_class(p)
