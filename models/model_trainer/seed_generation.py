def generate_seeds(kmer_length: int, pattern_length: int) -> list[str]:
    seeds = []
    for i in range(pattern_length):
        end = "1" + "0" * (pattern_length - i)
        gap = "0" * (i + 1 + i % 2)
        cares = "1" * (kmer_length // 2 - len(end) - len(gap) // 2)
        seed = end + cares + gap + cares + end[::-1]
        seed = seed[:len(seed) // 2] + seed[len(seed) // 2 + 1 - kmer_length % 2:]
        seeds.append(seed)
    return seeds


if __name__ == "__main__":
    print(*generate_seeds(71, 10), sep='\n')
