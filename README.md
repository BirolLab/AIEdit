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

- [btllib](https://github.com/bcgsc/btllib) [![install with bioconda](https://img.shields.io/badge/install%20with-bioconda-brightgreen.svg?style=flat)](http://bioconda.github.io/recipes/btllib/README.html)
- [catch2](https://github.com/catchorg/Catch2) (optional, required only for running tests) [![install with conda](https://anaconda.org/conda-forge/catch2/badges/installer/conda.svg)](https://anaconda.org/conda-forge/catch2)

No need to install, available in the `vendor` folder as git submodules:
- [argparse](https://github.com/p-ranav/argparse)
- [json](https://github.com/nlohmann/json)

# Compilation

First, clone the repo and `cd` into the `AIEdit` folder:

```shell
git clone --recurse-submodules git@github.com:bcgsc/AIEdit.git
cd AIEdit
```

Create a directory named `build` for compiling and installing AIEdit:

```shell
meson build
```

Finally, build the project:

```shell
cd build
ninja
```

This will create the executable `ai-edit` in the `build` directory.

# Run Tests

If [catch2](https://github.com/catchorg/Catch2) is installed, AIEdit's unit tests can be run by executing `ninja test` in the `build` directory.

# Usage

```
Usage: AIEdit [options] 

Artificially-intelligent long read genome polisher

Optional arguments:
  -h, --help          	shows help message and exits 
  -v, --version       	prints version information and exits 
  -a, --assembly      	Path to assembly file [required]
  -b, --bloom-filter  	Path to btllib SeedBloomFilter populated with reads and seeds [required]
  --long-mode         	Optimize seq. reader for long data (>5kbp) 
  -o, --out-path      	Path to output directory for storing results [default: "."]
  -V                  	Level of details printed to stdout 
  -w, --pattern-length	Number of bases to scan for errors after each detection [default: 5]
```

Use [ntHits](https://github.com/bcgsc/ntHits/tree/refactor) to generate the Bloom filter (for AIEdit's `-b` argument).

AIEdit will read the necessary parameters, such as the spaced seed patterns, from the Bloom filter file.

Example:

```shell
ntHits -h 1 -c 1 --outbloom -s 111001101100111,101010101010101,111100101001111,1100101111010011 reads_1.fa reads_2.fa
ai-edit -a draft.fa -b repeats_k15.bf
```

# Output

Information about the Bloom filter and the time elapsed for each step of the algorithm are printed to stdout.

In the output folder (specified by `-o`), the following files are created:

- `db.json`, a dump of the pattern database in JSON format
- `edited.fa`, the edited sequences in FASTA format
- `variants.vcf`, list of edits in VCF format
