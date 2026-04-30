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
  - [For Developers](#for-developers)
- [Usage](#usage)
  - [Models](#models)
- [Output Files](#output-files)
- [Running Tests](#running-tests)
- [License](#license)


# Requirements

- C++ compiler with C++17 support
- Python 3.10+
- [cmake](https://cmake.org/)
- [btllib](https://github.com/BirolLab/btllib)
- [libtorch for CPU](https://pytorch.org/)
- [pybind11](https://github.com/pybind/pybind11)
- [scikit-build-core](https://github.com/scikit-build/scikit-build-core)
- [ntStat](https://github.com/BirolLab/ntStat)
- [ntCard](https://github.com/BirolLab/ntCard)
- [ntEdit](https://github.com/BirolLab/ntEdit)


The `environment.yaml` file contains all the necessary dependencies for compiling AIEdit manually in a Conda environment:

```shell
conda env create -f environment.yaml
conda activate aiedit
```

If you would like to train new models:
- [PyTorch](https://pytorch.org/)
- [torchinfo](https://github.com/TylerYep/torchinfo)
- [tqdm](https://github.com/tqdm/tqdm)

# Installation

## Using `conda` (recommended)

AIEdit is available on Bioconda:

```shell
conda install bioconda::aiedit
```

This will make the `aiedit` command available in the environment.

## Manually

Build AIEdit in the `build` folder by running the following in the project's root folder:

```shell
pip install . --no-build-isolation
```

## For Developers

If you are modifying the C++ or Python code and want your changes to reflect immediately, use an editable install:

```shell
pip install -e . --no-build-isolation
```

Install [pybind11-stubgen](https://github.com/sizmailov/pybind11-stubgen) so the aiedit/core.pyi file will be updated in case of changes in the C++ bindings.

# Usage

AIEdit will run all required polishing stages given a set of reads `READS` and an assembly `ASSEMBLY`. Results will be stored in the output path specified by `-o`, which is the current working directory by default:

```shell
aiedit polish -r READS -a ASSEMBLY
```

Run `aiedit polish --help` for more details on the input parameters.

For polishing assemblies with ONT reads, we suggest setting `-y 10 -p 0.8`.

AIEdit uses half of the available CPUs on the machine by default. This can be adjusted with the `-t` parameter.

## Models

To list available pretrained models with their configurations, run:

```shell
aiedit list_models
```

The default model supports 5bp edit windows using 3 spaced seeds (`aiedit/pretrained/s3m5i5.pt`). More models are available in the `pretrained` directory. Additionally, new models can be trained using the `aiedit train` command. We recommend using the default model for balanced computational performance and polishing accuracy—feel free to train and experiment with other models.

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
