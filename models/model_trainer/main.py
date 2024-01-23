import os
import numpy as np

from args import parse_args
from data import prepare_data
from model import build_model, train_model, save_model
from plot import plot_training_stats

LOGO = (
    "           _____ ______    _ _ _            \n"
    "     /\\   |_   _|  ____|  | /_\\ |         \n"
    "    /  \\    | | | |__   __| | | |_         \n"
    "   / /\\ \\   | | |  __| / _` | | __|       \n"
    "  / ____ \\ _| |_| |___| (_| | | |_         \n"
    " /_/    \\_\\_____|______\\__,_|_|\\__|"
)

TRAINER_VERSION = "0.2"


def main():
    args = parse_args()
    print(LOGO, f"model-trainer v{TRAINER_VERSION}", sep="\t", end=os.linesep * 2)
    print(
        f"Training for w={args.w} and {len(args.s)} spaced seeds (k={len(args.s[0])}):"
    )
    print(*args.s, sep=os.linesep)
    print()
    model = build_model(args.s, args.w, TRAINER_VERSION)
    model.summary()
    print()
    data = prepare_data(args.s, args.w, args.n, args.fpr)
    _, counts = np.unique(data.patterns, return_counts=True)
    print(f"Training data size: {len(data.x_train)}")
    print(f"Class population mean = {counts.mean():.2f}, std = {counts.std():.2f}")
    print(f"Testing data size: {len(data.x_test)}")
    print()
    print("Training model")
    training_stats = train_model(model, data, args.e)
    model_file_name = args.o or model.name + ".json"
    save_model(model, args.w, args.s, model_file_name)
    print("Model saved to", model_file_name)
    if args.plot_stats:
        plot_training_stats(training_stats, args.o)


if __name__ == "__main__":
    main()
