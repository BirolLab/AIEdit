import os
import json
from tensorflow.keras import Model
from tensorflow.keras.layers import Input, Flatten, Dense
from tensorflow.keras.losses import CategoricalCrossentropy
from tensorflow.keras.metrics import CategoricalAccuracy
from tensorflow.keras.optimizers import Adam
import numpy as np
from datetime import datetime
from tqdm.keras import TqdmCallback

from data import Dataset
from fdeep_convert import convert


def get_model_name(seeds: list[str], pattern_length: int, version: str) -> str:
    return f"{hex(hash(''.join(seeds)))[-6:]}-w{pattern_length}-v{version}-{datetime.today().strftime('%Y_%m_%d')}"


def build_model(seeds: list[str], pattern_length: int, version: str) -> Model:
    x_in = Input((len(seeds[0]), len(seeds)))
    z_flat = Flatten()(x_in)
    y_out = Dense(2**pattern_length, activation="softmax")(z_flat)
    return Model(
        x_in, y_out, name=get_model_name(seeds, pattern_length, version)
    )


def train_model(model: Model, data: Dataset, num_epochs: int):
    model.compile(
        optimizer=Adam(learning_rate=0.001),
        loss=CategoricalCrossentropy(),
        metrics=[CategoricalAccuracy()],
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
