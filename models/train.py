import argparse
import dataclasses
import itertools
import os
import random
import re
import sys

import keras
import keras.layers
import keras.losses
import keras.metrics
import keras.optimizers
import matplotlib.pyplot as plt
import numpy as np


@dataclasses.dataclass
class Dataset:
    x_train: list[np.array]
    y_train: list[np.array]
    x_test: list[np.array]
    y_test: list[np.array]


def get_model(seeds: list[str], pattern_length: int) -> keras.Sequential:
    signature_length = pattern_length + len(seeds[0]) - 1
    return keras.Sequential([
        keras.layers.Conv1D(1,
                            pattern_length,
                            input_shape=(signature_length, len(seeds))),
        keras.layers.Flatten(),
        keras.layers.Dense(pattern_length, activation='sigmoid')
    ])


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", help="directory to store results", default=".")
    parser.add_argument("-w",
                        help="maximum pattern length",
                        default=5,
                        type=int)
    parser.add_argument("-e",
                        help="number of training epochs",
                        default=10,
                        type=int)
    parser.add_argument("-a",
                        help="Data augmentation ratio",
                        default=0.1,
                        type=float)
    parser.add_argument(
        "seeds",
        help="spaced seed patterns"
        "(or path to text file with seeds in separate lines)",
        nargs="+",
    )
    args = parser.parse_args()
    if not re.match(r"^[01]+$", args.seeds[0]):
        with open(args.seeds[0]) as f:
            args.seeds = list(map(str.strip, f.readlines()))
    if len(set(map(len, args.seeds))) != 1:
        msg = "Seed patterns should be the same length"
        print(msg, file=sys.stderr)
        exit(1)
    return args


def get_pattern_strings(pattern_length: int) -> list[str]:
    pattern_strings = []
    for i in range(2**(pattern_length - 1)):
        pattern_strings.append("1" + bin(i)[2:].zfill(pattern_length - 1))
    return pattern_strings


def get_signature(seeds: list[str], pattern_string: str) -> np.array:
    seed_length = len(seeds[0])
    padding = "0" * (seed_length - 1)
    pattern = padding + pattern_string + padding
    signature_length = len(pattern_string) + len(padding)
    signature = np.ones((signature_length, len(seeds)))
    for i, j in itertools.product(range(signature_length), range(len(seeds))):
        has_miss = False
        for pos in range(seed_length):
            if seeds[j][pos] == "1" and pattern[i + pos] == "1":
                has_miss = True
        if has_miss:
            signature[i][j] = 0
    return signature


def get_pattern_tensor(pattern_string: str) -> np.array:
    pattern = list(map(float, pattern_string))
    return np.expand_dims(np.array(pattern), axis=1)


def augment_data(data: Dataset, ratio: float) -> None:
    x_train, y_train, x_test, y_test = [], [], [], []
    for x, y in zip(data.x_train, data.y_train):
        miss_positions = []
        for i, j in itertools.product(range(x.shape[0]), range(x.shape[1])):
            if x[i][j] == 0.0:
                miss_positions.append((i, j))
        random.shuffle(miss_positions)
        miss_positions = miss_positions[:int(len(miss_positions) * ratio * 2)]
        for_train = True
        for i, j in miss_positions:
            x_c, y_c = x.copy(), y.copy()
            x_c[i][j] = 1.0
            if for_train:
                x_train.append(x_c)
                y_train.append(y_c)
            else:
                x_test.append(x_c)
                y_test.append(y_c)
            for_train = not for_train
    data.x_train.extend(x_train)
    data.y_train.extend(y_train)
    data.x_test.extend(x_test)
    data.y_test.extend(y_test)


def prepare_data(seeds: list[str], pattern_length: int,
                 augmentation_ratio: float) -> Dataset:
    pattern_strings = get_pattern_strings(pattern_length)
    x_train = [get_signature(seeds, p) for p in pattern_strings]
    y_train = [get_pattern_tensor(p) for p in pattern_strings]
    data = Dataset(x_train, y_train, [], [])
    augment_data(data, augmentation_ratio)
    return data


def get_validation_error(model: keras.Model, data: Dataset) -> float:
    num_accurate = 0
    for x, y_true in zip(data.x_test, data.y_test):
        y_pred = np.round(model.predict(x))
        if np.array_equal(y_pred, y_true):
            num_accurate += 1
    return 1 - num_accurate / len(data.x_test)


def train(model: keras.Model, data: Dataset, num_epochs: int):
    model.compile(optimizer=keras.optimizers.Adam(learning_rate=0.001),
                  loss=keras.losses.BinaryCrossentropy(),
                  metrics=[keras.metrics.BinaryAccuracy()])
    x_train, y_train = np.array(data.x_train), np.array(data.y_train)
    val = (np.array(data.x_test), np.array(data.y_test))
    training = model.fit(x_train,
                         y_train,
                         batch_size=1,
                         epochs=num_epochs,
                         validation_data=val)
    return training.history


def plot_training_stats(stats: dict, out_path: str) -> None:
    fig, ax = plt.subplots(1, 2, figsize=(8, 4), dpi=300)
    x = np.arange(len(stats['loss']))
    ax[0].plot(stats['loss'])
    ax[0].set_xticks(x, x + 1)
    ax[0].set_xlabel("Epoch")
    ax[0].set_ylabel("Training loss")
    ax[1].plot(stats['val_binary_accuracy'])
    ax[1].set_xticks(x, x + 1)
    ax[1].set_xlabel("Epoch")
    ax[1].set_ylabel("Validation accuracy")
    fig.tight_layout()
    plt.savefig(os.path.join(out_path, "training.png"))


def main():
    args = parse_args()
    model = get_model(args.seeds, args.w)
    model.summary()
    print("Preparing data... ", end="", flush=True)
    data = prepare_data(args.seeds, args.w, args.a)
    print("\b\b\b\b DONE")
    print(f"  - Training samples: {len(data.x_train)}")
    print(f"  - Testing samples: {len(data.x_test)}")
    training_stats = train(model, data, args.e)
    print("Training DONE")
    print("Saving results... ", end="", flush=True)
    model.save(os.path.join(args.o, 'model.h5'), include_optimizer=False)
    plot_training_stats(training_stats, args.o)
    print("\b\b\b\b DONE")
    print(data.x_test[0])
    print(model.predict(np.array([data.x_test[0]])))


if __name__ == "__main__":
    main()
