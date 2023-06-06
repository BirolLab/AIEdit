import argparse
import dataclasses
import json
import os
import re
import subprocess
import sys

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'

from collections import Counter

import keras
import keras.layers
import keras.losses
import keras.metrics
import keras.optimizers
import matplotlib.pyplot as plt
import numpy as np
import tensorflow as tf
from signature import get_signature, to_string
from sklearn.utils import class_weight

DEFAULT_SEEDS = [
    '10111111111111100000111111111111101',
    '10011111111111100000111111111111001',
    '10001111111111110001111111111110001',
    '10000111111111110001111111111100001',
    '10000011111111111011111111111000001'
]


@dataclasses.dataclass
class Dataset:
    x_train: np.array
    y_train: np.array
    x_test: np.array
    y_test: np.array
    patterns: list[str]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser("AIEdit pattern detector training script")
    parser.add_argument("-o", help="output model file path", required=True)
    parser.add_argument("-w",
                        help="maximum pattern length",
                        default=5,
                        type=int)
    parser.add_argument("-e",
                        help="number of training epochs",
                        default=50,
                        type=int)
    parser.add_argument("-n",
                        help="number of samples per class",
                        default=50,
                        type=int)
    parser.add_argument("-fpr",
                        help="false positive rate for simulation",
                        default=0.001,
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


def build_model(seeds: list[str], pattern_length: int) -> keras.Model:
    signature_length = pattern_length + len(seeds[0]) - 1
    num_patterns = 2**(pattern_length - 1) + 2 * pattern_length
    x_in = keras.layers.Input((signature_length, len(seeds)))
    z_conv = keras.layers.Conv1D(1, pattern_length)(x_in)
    z_flat = keras.layers.Flatten()(z_conv)
    y_out = keras.layers.Dense(num_patterns, activation='softmax')(z_flat)
    return keras.Model(x_in, y_out)


def train(model: keras.Model, data: Dataset, num_epochs: int):
    model.compile(optimizer=keras.optimizers.Adam(learning_rate=0.001),
                  loss=keras.losses.CategoricalCrossentropy(),
                  metrics=[keras.metrics.CategoricalAccuracy()])
    w = class_weight.compute_class_weight('balanced',
                                          classes=np.unique(data.y_train),
                                          y=data.y_train)
    w = dict(zip(np.unique(data.y_train), w))
    y_train = keras.utils.to_categorical(data.y_train)
    y_test = keras.utils.to_categorical(data.y_test)
    training = model.fit(data.x_train,
                         y_train,
                         batch_size=1,
                         epochs=num_epochs,
                         class_weight=w,
                         validation_data=(data.x_test, y_test))
    return training.history


def get_pattern_strings(pattern_length: int) -> list[str]:
    pattern_strings = []
    for i in range(2**(pattern_length - 1), 2**pattern_length):
        p_str = np.base_repr(i, 2).zfill(pattern_length).replace("1", "M")
        pattern_strings.append(p_str)
    for i in range(1, pattern_length + 1):
        pattern_strings.append("I" * i + "0" * (pattern_length - i))
        pattern_strings.append("D" * i + "0" * (pattern_length - i))
    return pattern_strings


def prepare_data(seeds: list[str], pattern_length: int, samples_per_class: int,
                 fpr: float) -> Dataset:
    generated = set()
    x_train, y_train, x_test, y_test = [], [], [], []
    patterns = get_pattern_strings(pattern_length)
    for y, pattern in enumerate(patterns):
        for i in range(max(round(samples_per_class * 1.2), 2)):
            signature = get_signature(seeds, pattern, fpr if i > 0 else 0)
            signature_string = to_string(signature, sep=',')
            if signature_string not in generated:
                generated.add(signature_string)
                if i < samples_per_class:
                    x_train.append(signature)
                    y_train.append(y)
                else:
                    x_test.append(signature)
                    y_test.append(y)
    x_train = np.array(x_train)
    y_train = np.array(y_train)
    x_test = np.array(x_test)
    y_test = np.array(y_test)
    return Dataset(x_train, y_train, x_test, y_test, patterns)


def save_model(model: keras.Model, seeds: list[str], patterns: list[str],
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
    json_data['seeds'] = seeds
    json_data['patterns'] = patterns
    with open(out_path, 'w') as json_file:
        json.dump(json_data, json_file, indent=4)


def plot_training_stats(stats: dict, out_path: str) -> None:
    fig, ax = plt.subplots(1, 2, figsize=(8, 4), dpi=300)
    ax[0].plot(stats[f'loss'], label='Train')
    ax[0].plot(stats[f'val_loss'], label=f'Test')
    ax[0].set_xlabel("Epoch")
    ax[0].set_ylabel("Loss")
    ax[0].legend()
    ax[1].plot(stats[f'categorical_accuracy'], label=f'Train')
    ax[1].plot(stats[f'val_categorical_accuracy'], label=f'Test')
    ax[1].set_xlabel("Epoch")
    ax[1].set_ylabel("Accuracy")
    ax[1].legend()
    fig.tight_layout()
    plt.savefig(os.path.join(os.path.dirname(out_path), "training.png"))


def print_class_counts(y: np.array, patterns: list[str]) -> None:
    for i, n in Counter(y.tolist()).items():
        print(patterns[i], n)


def main():
    args = parse_args()
    model = build_model(args.seeds, args.w)
    model.summary()
    data = prepare_data(args.seeds, args.w, args.n, args.fpr)
    print(f"Training samples: {len(data.x_train)}")
    print_class_counts(data.y_train, data.patterns)
    print(f"Testing samples: {len(data.x_test)}")
    print_class_counts(data.y_test, data.patterns)
    training_stats = train(model, data, args.e)
    print("Saving model... ")
    save_model(model, args.seeds, data.patterns, args.o)
    if args.plot_stats:
        print("Plotting stats...")
        plot_training_stats(training_stats, args.o)
    print(args.o, "successfully trained")


if __name__ == "__main__":
    main()
