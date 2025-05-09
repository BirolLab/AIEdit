import os

from aiedit import core


def main(args):
    seed_generator = core.SeedGenerator(100, 100, 0.5)
    print("Generating seeds... ", end="")
    seeds = seed_generator.generate(
        args.num_seeds, args.kmer_size, args.max_mismatches, args.max_indels
    )
    print("DONE")
    seeds = sorted(seeds, key=lambda seed: seed.count("0"))
    with open(args.out_file, "w") as fp:
        fp.write(os.linesep.join(seeds))
