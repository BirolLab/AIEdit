#include "sequence_iterator.hpp"

namespace aiedit {

SequenceIterator::HashVector SequenceIterator::to_hash_vector(const uint64_t* nthash_hashes)
{
    HashVector hashes;
    for (unsigned i = 0; i < seeds.size(); i++) {
        std::vector<uint64_t> seed_hashes;
        seed_hashes.reserve(hash_fn.get_hash_num_per_seed());
        const unsigned i_begin = i * hash_fn.get_hash_num_per_seed();
        const unsigned i_end = i_begin + hash_fn.get_hash_num_per_seed();
        seed_hashes.insert(seed_hashes.end(), nthash_hashes + i_begin, nthash_hashes + i_end);
        hashes.push_back(seed_hashes);
    }
    return hashes;
}

void SequenceIterator::next(unsigned n)
{
    for (unsigned i = 0; i < n && has_next(); i++) {
        hash_fn.roll();
    }
}

void SequenceIterator::previous(unsigned n)
{
    for (unsigned i = 0; i < n && has_next(); i++) {
        hash_fn.roll_back();
    }
}

bool SequenceIterator::has_next() { return get_position() < end - hash_fn.get_k(); }

char SequenceIterator::get_base(size_t position) { return seq[position]; }

size_t SequenceIterator::get_position() { return hash_fn.get_pos() + hash_fn.get_k() - 1; }

SequenceIterator::HashVector SequenceIterator::get_hashes()
{
    return to_hash_vector(hash_fn.hashes());
}

const std::string& SequenceIterator::get_sequence() { return seq; }

std::vector<SequenceIterator::HashVector> SequenceIterator::peek_hashes(unsigned window_size)
{
    nthash::SeedNtHash h_copy(hash_fn);
    std::vector<HashVector> hashes;
    hashes.reserve(window_size);
    hashes.push_back(to_hash_vector(h_copy.hashes()));
    while (--window_size > 0 && h_copy.roll()) {
        hashes.push_back(to_hash_vector(h_copy.hashes()));
    }
    return hashes;
}

unsigned SequenceIterator::get_seed_length() { return seeds[0].size(); }

unsigned SequenceIterator::get_num_seeds() { return seeds.size(); }

void SequenceIterator::update(size_t position, char value)
{
    seq[position] = value;
    hash_fn.change_seq(seq, hash_fn.get_pos());
}

void SequenceIterator::insert(size_t position, char value)
{
    seq.insert(position, 1, value);
    hash_fn.change_seq(seq, hash_fn.get_pos());
}

void SequenceIterator::insert(size_t position, std::string bases)
{
    seq.insert(position, bases);
    hash_fn.change_seq(seq, hash_fn.get_pos());
}

void SequenceIterator::remove(size_t position)
{
    seq.erase(position, 1);
    hash_fn.change_seq(seq, hash_fn.get_pos());
}

}  // namespace aiedit