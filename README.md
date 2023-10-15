# AIEdit: Artificially-intelligent long read genome polisher

```
           _____ ______    _ _ _            
     /\   |_   _|  ____|  | /_\ |          
    /  \    | | | |__   __| | | |_         
   / /\ \   | | |  __| / _` | | __|       
  / ____ \ _| |_| |___| (_| | | |_         
 /_/    \_\_____|______\__,_|_|\__|
 ```
 
# Dependencies

- C++ compiler with c++17 and OpenMP support
- [Meson](https://mesonbuild.com/)
- [btllib](https://github.com/bcgsc/btllib)
- [FunctionalPlus](https://github.com/Dobiasd/FunctionalPlus)
- [ntHash](https://github.com/bcgsc/ntHash)
- [catch2](https://github.com/catchorg/Catch2) (optional, required only for running tests)

No need to install, available in the `vendor` folder as git submodules:
- [argparse](https://github.com/p-ranav/argparse)
- [json](https://github.com/nlohmann/json)
- [frugally-deep](https://github.com/Dobiasd/frugally-deep)
- [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)

# Compilation

First, clone the repo and `cd` into the `AIEdit` folder:

```shell
git clone --recurse-submodules git@github.com:bcgsc/AIEdit.git
cd AIEdit
```

Create a directory named `build` for compiling and installing AIEdit:

```shell
meson setup build
```

Finally, build the project:

```shell
meson compile -C build
```

This will create the executable `aiedit` in the `build` directory.

# Run Tests

If [catch2](https://github.com/catchorg/Catch2) is installed, AIEdit's unit tests can be run by executing `ninja test` in the `build` directory.

# Usage

```
Usage: AIEdit [options] input_file

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

Use [ntHits](https://github.com/bcgsc/ntHits) to generate the Bloom filter (for AIEdit's `-b` argument).

AIEdit will read the necessary parameters, such as the spaced seed patterns, from the Bloom filter file.

Example:

```shell
ntHits -h 1 -c 1 --outbloom -s 111001101100111,101010101010101,111100101001111,1100101111010011 reads_1.fa reads_2.fa
ai-edit -a draft.fa -b repeats_k15.bf
```

# Output files

The following files are created in the output folder (specified by `-o`, `<input_file>` is replaced by the file name given as input to the program):

- `<input_file>-aiedit-polished.fa`, polished data in FASTA format
- `<input_file>-aiedit-variants.vcf`, list of edits in VCF format
- `<input_file>-aiedit-ignored.tsv`, list of detected error patterns which AIEdit failed to fix
