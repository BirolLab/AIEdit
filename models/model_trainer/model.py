import json
import os

import keras
from data import Dataset
from fdeep_convert import convert


class MismatchDetector:
    def __init__(
        self, seeds: list[str], signature_length: int, pattern_length: int
    ) -> None:
        super().__init__()
        self.__seeds = seeds
        self.__signature_length = signature_length
        self.__pattern_length = pattern_length
        self.__model = keras.models.Sequential(
            [
                keras.layers.Input((signature_length, len(seeds))),
                keras.layers.Flatten(),
                keras.layers.Dense(pattern_length, activation="sigmoid"),
            ]
        )
        self.__model.compile(
            optimizer=keras.optimizers.Adam(learning_rate=0.001),
            loss=keras.losses.BinaryCrossentropy(),
            metrics=[keras.metrics.BinaryAccuracy()],
        )

    @property
    def seeds(self) -> list[str]:
        return self.__seeds

    @property
    def signature_length(self) -> int:
        return self.__signature_length

    @property
    def pattern_length(self) -> int:
        return self.__pattern_length

    def print_summary(self) -> None:
        return self.__model.summary()

    def train(self, data: Dataset, num_epochs: int):
        training = self.__model.fit(
            data.x_train,
            data.y_train,
            batch_size=1,
            epochs=num_epochs,
            validation_data=(data.x_test, data.y_test),
        )
        return training.history

    def save(self, path):
        temp_file = path + ".keras"
        self.__model.save(temp_file)
        convert(temp_file, path, True)
        os.remove(temp_file)
        with open(path) as json_file:
            json_data = json.load(json_file)
        json_data["signature_length"] = self.__signature_length
        json_data["pattern_length"] = self.__pattern_length
        json_data["seeds"] = self.__seeds
        with open(path, "w") as json_file:
            json.dump(json_data, json_file, indent=4)
