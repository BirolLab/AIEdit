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
from signature import get_signature
from seed_generation import generate_seeds
from tqdm import tqdm


@dataclasses.dataclass
class Dataset:
    x_train: list[np.array]
    y_train: list[np.array]
    x_test: list[np.array]
    y_test: list[np.array]
    patterns: list[str]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser("AIEdit pattern detector training script")
    parser.add_argument("-s",
                        help="spaced seed patterns separated by commas "
                        "(e.g., 1110111,1101011), or k-mer length (e.g., 64)",
                        nargs="+",
                        required=True)
    parser.add_argument("-w",
                        help="maximum pattern length",
                        default=5,
                        type=int)
    parser.add_argument("-e",
                        help="number of training epochs",
                        default=20,
                        type=int)
    parser.add_argument("-n",
                        help="number of samples per class",
                        default=20,
                        type=int)
    parser.add_argument("-fpr",
                        help="false positive rate for simulation",
                        default=0.001,
                        type=float)
    parser.add_argument("-o",
                        help="output model file path",
                        default="model.json")
    parser.add_argument('--plot-stats',
                        action='store_true',
                        help="plot training stats"
                        " (stored in training.png next to the model)")
    args = parser.parse_args()
    if not re.match(r"^[01]+$", args.s[0]):
        args.s = generate_seeds(int(args.s[0]), args.w)
    if len(set(map(len, args.s))) != 1:
        msg = "Seed patterns should be the same length"
        print(msg, file=sys.stderr)
        exit(1)
    return args


def build_model(seeds: list[str], pattern_length: int) -> keras.Model:
    signature_length = pattern_length + len(seeds[0]) - 1
    x_in = keras.layers.Input((signature_length, len(seeds)))
    z_flat = keras.layers.Flatten()(x_in)
    y_out = keras.layers.Dense(2 ** pattern_length, activation='softmax')(z_flat)
    return keras.Model(x_in, y_out)


def train(model: keras.Model, data: Dataset, num_epochs: int):
    model.compile(optimizer=keras.optimizers.Adam(learning_rate=0.001),
                  loss=keras.losses.CategoricalCrossentropy(),
                  metrics=[keras.metrics.CategoricalAccuracy()])
    x_train = np.array(data.x_train)
    y_train = np.array(data.y_train)
    x_val = np.array(data.x_test)
    y_val = np.array(data.y_test)
    training = model.fit(x_train,
                         y_train,
                         batch_size=1,
                         epochs=num_epochs,
                         validation_data=(x_val, y_val))
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


def get_pattern_tensor(pattern_string: str) -> np.array:
    pattern = np.zeros(2 ** len(pattern_string))
    i1 = 0
    for i in range(len(pattern_string)):
        if pattern_string[i] == 'M':
            i1 += 2 ** i
    pattern[i1] = 1.0
    return pattern


def prepare_data(seeds: list[str], pattern_length: int, samples_per_class: int,
                 fpr: float) -> Dataset:
    x_train, y_train, x_test, y_test = [], [], [], []
    patterns = []
    for p in tqdm(get_pattern_strings(pattern_length), desc="GENERATING DATA", unit="pattern"):
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
    ax[0].plot(stats[f'loss'], label="Training")
    ax[0].plot(stats[f'val_loss'], label="Validation")
    ax[1].plot(stats[f'categorical_accuracy'], label="Training")
    ax[1].plot(stats[f'val_categorical_accuracy'], label="Validation")
    ax[0].set_xlabel("Epoch")
    ax[0].set_ylabel("Loss")
    ax[0].legend()
    ax[1].set_xlabel("Epoch")
    ax[1].set_ylabel("Accuracy")
    ax[1].legend()
    fig.tight_layout()
    plt.savefig(os.path.join(os.path.dirname(out_path), "training.png"))


def main():
    args = parse_args()
    print(f"Training for w={args.w} and {len(args.s)} spaced seeds:")
    print(*args.s, sep=os.linesep)
    model = build_model(args.s, args.w)
    model.summary()
    data = prepare_data(args.s, args.w, args.n, args.fpr)
    print(f"Training samples: {len(data.x_train)}")
    for c, n in Counter(data.patterns).items():
        print(c.replace("0", "-"), n)
    print(f"Testing samples: {len(data.x_test)}")
    training_stats = train(model, data, args.e)
    save_model(model, args.w, args.s, args.o)
    if args.plot_stats:
        plot_training_stats(training_stats, args.o)


if __name__ == "__main__":
    main()