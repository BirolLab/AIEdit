def generate_seeds(kmer_length: int, pattern_length: int) -> list[str]:
    seeds = []
    for i in range(1, pattern_length + 1):
        gap = "0" * i + "1" + "0" * i
        cares = "1" * ((kmer_length - len(gap)) // 2)
        seeds.append(cares + gap + cares)
    return seeds


if __name__ == "__main__":
    print(*generate_seeds(63, 15), sep='\n')
