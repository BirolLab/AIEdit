import os
import matplotlib.pyplot as plt


def plot_training_stats(stats: dict, out_path: str) -> None:
    fig, ax = plt.subplots(1, 2, figsize=(8, 4), dpi=300)
    ax[0].plot(stats[f"loss"], label="Training")
    ax[0].plot(stats[f"val_loss"], label="Validation")
    ax[1].plot(stats[f"categorical_accuracy"], label="Training")
    ax[1].plot(stats[f"val_categorical_accuracy"], label="Validation")
    ax[0].set_xlabel("Epoch")
    ax[0].set_ylabel("Loss")
    ax[0].legend()
    ax[1].set_xlabel("Epoch")
    ax[1].set_ylabel("Accuracy")
    ax[1].legend()
    fig.tight_layout()
    plt.savefig(os.path.join(os.path.dirname(out_path), "training.png"))
