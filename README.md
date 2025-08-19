```
           _____ ______    _ _ _            
     /\   |_   _|  ____|  | /_\ |          
    /  \    | | | |__   __| | | |_         
   / /\ \   | | |  __| / _` | | __|       
  / ____ \ _| |_| |___| (_| | | |_         
 /_/    \_\_____|______\__,_|_|\__|
 ```

 Alignment-free genome assembly polisher with an ML model trained on spaced seed hit/miss patterns.

- [Requirements](#requirements)
- [Installation](#installation)
  - [Using `conda` (recommended)](#using-conda-recommended)
  - [Manually](#manually)
- [Usage](#usage)
- [Output Files](#output-files)
- [Running Tests](#running-tests)
- [License](#license)


# Requirements

- C++ compiler with C++17 support
- Python 3.10+
- [cmake](https://mesonbuild.com/)
- [btllib](https://github.com/bcgsc/btllib)
- [PyTorch and libtorch](https://pytorch.org/)
- [pybind11](https://github.com/pybind/pybind11)
- [ntStat](https://github.com/bcgsc/ntStat)
- [ntCard](https://github.com/bcgsc/ntCard)
- [ntEdit](https://github.com/bcgsc/ntEdit)

If you would like to train models too:
- [torchinfo](https://github.com/TylerYep/torchinfo)
- [tqdm](https://github.com/tqdm/tqdm)

# Installation

## Using `conda` (recommended)

AIEdit is available on Bioconda:

```shell
conda install bioconda::aiedit
```

## Manually

Build AIEdit in the `build` folder by running the following in the project's root folder:

```
cmake -S . -B build
cmake --build build
```

This will put a `core*.so` file in the `aiedit` package, which can now be used by adding the project root to `$PYTHONPATH` and running:

```shell
python -m aiedit
```

If PyTorch/libtorch are installed in a conda environment, you might have you update the `CMAKE_PREFIX_PATH` environment variable. To find PyTorch's CMake prefix path, run:

```shell
python -c "import torch; print(torch.utils.cmake_prefix_path)"
```

Then, pass the result to CMake:

```shell
cmake -DCMAKE_PREFIX_PATH=<TORCH_PREFIX_PATH> -S . -B build
cmake --build build
```

# Usage

AIEdit will run all required polishing stages given a set of reads `READS` and an assembly `ASSEMBLY`. Results will be stored in the output path specified by `-o`, which is the current working directory by default:

```
aiedit polish -r READS -a ASSEMBLY
```

Run `aiedit polish --help` for more details on the input parameters.

For polishing assemblies with ONT reads, we suggest setting `-y 10 -p 0.8`.

AIEdit uses half of the available CPUs on the machine by default. This can be adjusted with the `-t` parameter.

# Output Files

The following files are created in the output folder (specified by `-o`). `<input_file>` is replaced by the draft assembly file's name:

- `<input_file>-aiedit_edited.fa`, polished assembly in FASTA format
- `<input_file>-aiedit_variants.vcf`, list of AIEdit's changes
- `<input_file>-ntedit_variants.vcf`, list of ntEdit's changes

# Running Tests

After compiling the project manually in `build`, run:

```shell
ctest --testdir build/tests
```

# License

AIEdit Copyright (c) 2025-present British Columbia Cancer Agency Branch. All rights reserved.

AIEdit is released under the GNU General Public License v3

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see http://www.gnu.org/licenses/.

For commercial licensing options, please contact Patrick Rebstein prebstein@bccancer.bc.ca