import json
import os

import numpy as np
from data import Dataset
from fdeep_convert import convert
from keras import Model
from keras.layers import Dense, Flatten, Input
from keras.losses import BinaryCrossentropy
from keras.metrics import BinaryAccuracy
from keras.optimizers import Adam
from tqdm.keras import TqdmCallback


def build_model(seeds: list[str], pattern_length: int, version: str) -> Model:
    signature_length = len(seeds[0])
    x_in = Input((signature_length, len(seeds)))
    z_flat = Flatten()(x_in)
    y_out = Dense(pattern_length, activation="sigmoid")(z_flat)
    model_name = f"{hex(hash(''.join(seeds)))[-8:]}-w{pattern_length}-v{version}"
    return Model(x_in, y_out, name=model_name)


def train_model(model: Model, data: Dataset, num_epochs: int):
    model.compile(
        optimizer=Adam(learning_rate=0.001),
        loss=BinaryCrossentropy(),
        metrics=[BinaryAccuracy()],
    )
    x_train = np.array(data.x_train)
    y_train = np.array(data.y_train)
    x_val = np.array(data.x_test)
    y_val = np.array(data.y_test)
    training = model.fit(
        x_train,
        y_train,
        batch_size=1,
        epochs=num_epochs,
        validation_data=(x_val, y_val),
        callbacks=[TqdmCallback()],
        verbose=0,
    )
    return training.history


def save_model(model: Model, pattern_length: int, seeds: list[str], out_path: str):
    temp_file = out_path + ".keras"
    model.save(temp_file)
    convert(temp_file, out_path, True)
    os.remove(temp_file)
    with open(out_path) as json_file:
        json_data = json.load(json_file)
    json_data["pattern_length"] = pattern_length
    json_data["seeds"] = seeds
    with open(out_path, "w") as json_file:
        json.dump(json_data, json_file, indent=4)
