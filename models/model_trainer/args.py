import argparse
import os


class ProgramArguments:
    def __init__(self) -> None:
        self.__parser = argparse.ArgumentParser(
            "AIEdit pattern detector training script"
        )
        self.__parser.add_argument(
            "-s",
            help="path to text file containing one spaced seed pattern per line",
            required=True,
        )
        self.__parser.add_argument(
            "-w", help="maximum pattern length", default=5, type=int
        )
        self.__parser.add_argument(
            "-e", help="number of training epochs", default=20, type=int
        )
        self.__parser.add_argument(
            "-n", help="number of samples per class", default=20, type=int
        )
        self.__parser.add_argument(
            "-fpr", help="false positive rate for simulation", default=0.001, type=float
        )
        self.__parser.add_argument("-o", help="output model file path", required=False)
        self.__parser.add_argument(
            "--plots",
            action="store_true",
            help="plot training stats (saved with the model file)",
        )
        self.__parsed_args = self.__parser.parse_args()
        with open(self.__parsed_args.s) as fp:
            seeds = list(map(str.strip, fp.readlines()))
        if "1" * len(seeds[0]) in seeds:
            seeds.remove("1" * len(seeds[0]))
        self.__parsed_args.s = seeds
        if len(set(map(len, self.__parsed_args.s))) != 1:
            raise ValueError("Seed patterns (-s) should have the same length")

    @property
    def seeds(self) -> list[str]:
        return self.__parsed_args.s

    @property
    def pattern_length(self) -> int:
        return self.__parsed_args.w

    @property
    def num_epochs(self) -> int:
        return self.__parsed_args.e

    @property
    def num_samples_per_class(self) -> int:
        return self.__parsed_args.n

    @property
    def false_positive_rate(self) -> float:
        return self.__parsed_args.fpr

    @property
    def model_path(self) -> str:
        return self.__parsed_args.o

    @property
    def plot_stats(self) -> bool:
        return self.__parsed_args.plots

    def print_values(self):
        print("Pattern length:", self.pattern_length)
        print("Number of spaced seeds:", len(self.seeds))
        print("Spaced seed length:", len(self.seeds[0]))
        print(*self.seeds, sep=os.linesep)
