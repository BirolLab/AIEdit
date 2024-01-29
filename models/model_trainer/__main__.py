import os

from args import ProgramArguments
from data import Dataset
from model import MismatchDetector
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

print(LOGO, f"model-trainer v{TRAINER_VERSION}", sep="\t", end=os.linesep * 2)

args = ProgramArguments()
args.print_values()

model = MismatchDetector(args.seeds, args.signature_length, args.pattern_length)
model.print_summary()

dataset = Dataset(
    args.seeds,
    args.signature_length,
    args.pattern_length,
    args.num_samples_per_class,
    args.false_positive_rate,
)
dataset.print_details()

training_stats = model.train(dataset, args.num_epochs)
model.save(args.model_path)

if args.plot_stats:
    plot_training_stats(training_stats, args.model_path)
