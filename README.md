# AIEdit: Artificially-intelligent long read genome polisher

```
           _____ ______    _ _ _            
     /\   |_   _|  ____|  | /_\ |          
    /  \    | | | |__   __| | | |_         
   / /\ \   | | |  __| / _` | | __|       
  / ____ \ _| |_| |___| (_| | | |_         
 /_/    \_\_____|______\__,_|_|\__|
 ```

# Requirements

- C++ compiler with C++17 support
- [conan](https://conan.io/) (optional, recommended for installing dependencies)
- [btllib](https://github.com/bcgsc/btllib)
- [meson](https://mesonbuild.com/)
- [argparse](https://github.com/p-ranav/argparse)
- [json](https://github.com/nlohmann/json)
- [frugally-deep](https://github.com/Dobiasd/frugally-deep)

# Installation

## Using `conan` (recommended)

1. Setup your `conan` profile if required (e.g., when using `compilers` from `conda`)
1. Compile btllib (or install from `conda`) and set the necessary flags
1. Build AIEdit in the `build` folder by running the following in the project's root folder:

```shell
conan build . -of build
```

You can add any other arguments to the `conan` command accordingly.

## Manually

1. Install [meson](https://mesonbuild.com/) and the C++ dependencies. All are available on `conda` except [frugally-deep](https://github.com/Dobiasd/frugally-deep/blob/master/INSTALL.md). 
1. Build AIEdit in the `build` folder by running the following in the project's root folder:

```shell
meson setup build
meson compile -C build
```

# Running Tests

If [catch2](https://github.com/catchorg/Catch2) was available during compilation, you can run AIEdit's unit tests:

```shell
meson test -C build
```

# Usage

```
Usage: aiedit [options] input_file

Artificially-intelligent long read genome polisher

Positional arguments:
input_file        	path to input file

Optional arguments:
-h --help         	shows help message and exits [default: false]
-v --version      	prints version information and exits [default: false]
-b --bloom-filter 	path to ntHits counting Bloom filter file [required]
-m --model        	path to pattern detector model [required]
-o --out-path     	output directory for storing results [default: "."]
-t --num-threads  	number of threads to run in parallel [default: 1]
--contig-mode     	optimize multithreading for polishing contigs/reads [default: false]
--verbose         	print more details to stdout and log ignored patterns to ignored.tsv [default: false]
```

Use [ntHits](https://github.com/bcgsc/ntHits) to generate the Bloom filter (for AIEdit's `-b` argument). AIEdit will read the necessary parameters, such as the spaced seed patterns, from the Bloom filter file.

# Output Files

The following files are created in the output folder (specified by `-o`). `<input_file>` is replaced by the draft assembly file's name:

- `<input_file>-aiedit-polished.fa`, polished assembly in FASTA format
- `<input_file>-aiedit-variants.vcf`, list of edits as a VCF file
- `<input_file>-aiedit-ignored.tsv`, list of detected error patterns which AIEdit failed to fix
