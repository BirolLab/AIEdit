import os

from aiedit import core


def main(args):
    seed_generator = core.SeedGenerator(
        args.pop_size, args.num_gens, args.mutation, args.random_seed
    )
    print("Generating seeds... ", end="")
    seeds = seed_generator.generate(
        args.num_seeds, args.kmer_size, args.max_mismatches, args.max_indels
    )
    print("DONE")
    seeds = sorted(seeds, key=lambda seed: seed.count("0"))
    if args.out_file is None:
        print()
        print(*seeds, sep=os.linesep)
        return
    with open(args.out_file, "w") as fp:
        fp.write(os.linesep.join(seeds))
    print(f"Spaced seed patterns saved to {args.out_file}")
