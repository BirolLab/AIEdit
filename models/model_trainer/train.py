import argparse
import dataclasses
import itertools
import json
import os
import random
import re
import subprocess
import sys

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'

import keras
import keras.layers
import keras.losses
import keras.metrics
import keras.optimizers
import matplotlib.pyplot as plt
import numpy as np

from signature import get_signature

DEFAULT_SEEDS = [
    "1111111111110111111111111",
    "1111111111100011111111111",
    "1111111111000001111111111",
    "1111111111001001111111111",
    "1111111100001000011111111",
]


@dataclasses.dataclass
class Dataset:
    x_train: list[np.array]
    y_train: list[np.array]
    x_test: list[np.array]
    y_test: list[np.array]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser("AIEdit pattern detector training script")
    parser.add_argument("-o", help="output model file path", required=True)
    parser.add_argument("-w",
                        help="maximum pattern length",
                        default=5,
                        type=int)
    parser.add_argument("-e",
                        help="number of training epochs",
                        default=10,
                        type=int)
    parser.add_argument("-a",
                        help="data augmentation ratio",
                        default=0.1,
                        type=float)
    parser.add_argument("seeds",
                        help="spaced seed patterns"
                        " (or path to text file with seeds in separate lines)",
                        nargs="*",
                        default=DEFAULT_SEEDS)
    parser.add_argument('--plot-stats',
                        action='store_true',
                        help="plot training stats"
                        " (stored in training.png next to the model)")
    args = parser.parse_args()
    if not re.match(r"^[01]+$", args.seeds[0]):
        with open(args.seeds[0]) as f:
            args.seeds = list(map(str.strip, f.readlines()))
    if len(set(map(len, args.seeds))) != 1:
        msg = "Seed patterns should be the same length"
        print(msg, file=sys.stderr)
        exit(1)
    print(f"Training for w={args.w} and {len(args.seeds)} spaced seeds:")
    print(*args.seeds, sep=os.linesep)
    return args


def build_model(seeds: list[str], pattern_length: int) -> keras.Sequential:
    signature_length = pattern_length + len(seeds[0]) - 1
    return keras.Sequential([
        keras.layers.Conv1D(1,
                            pattern_length,
                            input_shape=(signature_length, len(seeds))),
        keras.layers.Flatten(),
        keras.layers.Dense(pattern_length * 3, activation='sigmoid')
    ])


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


def get_pattern_strings(pattern_length: int) -> list[str]:
    pattern_strings = []
    for i in range(2**pattern_length):
        p_str = np.base_repr(i, 2).zfill(pattern_length).replace("1", "M")
        pattern_strings.append(p_str)
    for i in range(1, pattern_length):
        pattern_strings.append("I" * i + "0" * (pattern_length - i))
        pattern_strings.append("D" * i + "0" * (pattern_length - i))
    return pattern_strings


def get_pattern_tensor(pattern_string: str) -> np.array:
    pattern = np.zeros((len(pattern_string) * 3, 1))
    for i in range(len(pattern_string)):
        if pattern_string[i] == 'M':
            pattern[3 * i] = 1.0
        elif pattern_string[i] == "I":
            pattern[3 * i + 1] = 1.0
        elif pattern_string[i] == "D":
            pattern[3 * i + 2] = 1.0
    return pattern


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


def save_model(model: keras.Model, pattern_length: int, seeds: list[str],
               out_path: str):
    model_temp_h5 = out_path + '.h5'
    model.save(model_temp_h5, include_optimizer=False)
    current_dir = os.path.dirname(__file__)
    project_dir = os.path.dirname(os.path.dirname(current_dir))
    script_path = os.path.join(project_dir, 'vendor', 'frugally-deep',
                               'keras_export', 'convert_model.py')
    args = ['python', script_path, model_temp_h5, out_path, '--no-tests']
    call_result = subprocess.call(args, stderr=subprocess.PIPE)
    if call_result != 0:
        print(call_result.stderr)
    else:
        os.remove(model_temp_h5)
    with open(out_path) as json_file:
        json_data = json.load(json_file)
    json_data['pattern_length'] = pattern_length
    json_data['seeds'] = seeds
    with open(out_path, 'w') as json_file:
        json.dump(json_data, json_file, indent=4)


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
    plt.savefig(os.path.join(os.path.dirname(out_path), "training.png"))


def main():
    args = parse_args()
    model = build_model(args.seeds, args.w)
    model.summary()
    data = prepare_data(args.seeds, args.w, args.a)
    print(f"Training samples: {len(data.x_train)}")
    print(f"Testing samples: {len(data.x_test)}")
    training_stats = train(model, data, args.e)
    save_model(model, args.w, args.seeds, args.o)
    if args.plot_stats:
        plot_training_stats(training_stats, args.o)


if __name__ == "__main__":
    main()
