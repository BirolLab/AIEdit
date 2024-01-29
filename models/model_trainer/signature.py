import random

import btllib
import numpy as np


def get_signature(seeds: list[str],
                  signature_length: int,
                  pattern: str,
                  fpr: float = 0,
                  verbose=False) -> np.array:
    seed_length = len(seeds[0])
    pattern_length = len(pattern)
    seq_length = 2 * seed_length + pattern_length - 2
    # generate reference sequence
    ref = ''.join(random.choice('ACGT') for _ in range(seq_length))
    bf = btllib.SeedBloomFilter(1024 * 1024, seed_length, seeds, 3)
    bf.insert(ref)
    # generate alt sequence
    alt = ref[seed_length - 1:seed_length + pattern_length - 1]
    j = 0
    for p in pattern:
        if p == 'M':
            b = random.choice('ACGT'.replace(alt[j], ''))
            alt = alt[:j] + b + alt[j + 1:]
            j += 1
        elif p == 'D':
            alt = alt[:j] + random.choice('ACGT') + alt[j:]
            j += 1
        elif p == 'I':
            alt = alt[:j] + alt[j + 1:]
        else:
            j += 1
    alt = (ref[:seed_length - 1], alt, ref[-seed_length + 1:])
    if verbose:
        print(ref[:seed_length - 1],
              ref[seed_length - 1:-seed_length + 1].zfill(len(alt[1])).replace(
                  '0', ' '),
              ref[-seed_length + 1:],
              sep='|')
        print(*alt, sep='|')
    alt = ''.join(alt)
    # create signature
    h = btllib.SeedNtHash(alt, btllib.parse_seeds(seeds),
                          bf.get_hash_num_per_seed(), seed_length)
    signature = np.empty((signature_length, len(seeds)))
    for i in range(signature.shape[0]):
        h.roll()
        for j in range(len(seeds)):
            begin = j * bf.get_hash_num_per_seed()
            end = (j + 1) * bf.get_hash_num_per_seed()
            hashes = h.hashes()[begin:end]
            signature[i][j] = 1.0 if bf.contains(hashes) else 0.0
            if random.uniform(0, 1) < fpr:
                signature[i][j] = 1.0
    return signature


def to_string(signature: np.array, sep='\n'):
    s = ''
    for i in range(signature.shape[1]):
        for j in range(signature.shape[0]):
            s += str(int((signature[j][i])))
        s += sep
    return s


if __name__ == "__main__":
    seeds = [
        '1011111111001001111111101',
        '1001111111000001111111001',
        '1000111111101011111110001',
        '1000011111100011111100001',
        '1000001111110111111000001'
    ]
    print(to_string(get_signature(seeds, 5, 'MMM--', verbose=True)))
