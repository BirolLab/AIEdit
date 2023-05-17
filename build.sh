#!/usr/bin/env bash

cmake_prefix_path=$(python -c "import torch;import os;print(os.path.join(torch.utils.cmake_prefix_path, 'Torch'))")

meson setup build --buildtype=release --cmake-prefix-path $cmake_prefix_path $1
cd build
ninja 
