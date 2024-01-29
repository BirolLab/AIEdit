import argparse
import btllib
import tqdm
import numpy as np


arg_parser = argparse.ArgumentParser()
arg_parser.add_argument("-a", help="path to assembly file", required=True)
arg_parser.add_argument("-b", help="path to bloom filter file", required=True)
arg_parser.add_argument("-k", help="k-mer length", type=int, required=True)
args = arg_parser.parse_args()

seq_reader = btllib.SeqReader(args.a, btllib.SeqReaderFlag.LONG_MODE)
bf = btllib.CountingBloomFilter8(args.b)

distances = []
for record in seq_reader:
    hash_fn = btllib.NtHash(record.seq, bf.get_hash_num(), args.k)
    progress_bar = tqdm.tqdm(total=len(record.seq) - args.k, unit="bp", leave=False)
    progress_bar.set_description(record.id)
    while hash_fn.roll():
        hit = True
        dist = 0
        pos = hash_fn.get_pos()
        while hit and hash_fn.roll():
            hit = bf.contains(hash_fn.hashes()) > 0
        progress_bar.update(hash_fn.get_pos() - pos)
        pos = hash_fn.get_pos()
        while not hit and hash_fn.roll():
            hit = bf.contains(hash_fn.hashes()) > 0
            dist += 1
        progress_bar.update(hash_fn.get_pos() - pos)
        if dist > args.k:
            distances.append(dist)
    progress_bar.update(progress_bar.total - progress_bar.n)
    progress_bar.close()

ptile = np.percentile(distances, 75)
print(int(ptile) - args.k + 1)
