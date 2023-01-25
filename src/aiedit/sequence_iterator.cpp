#include "aiedit/sequence_iterator.hpp"

namespace aiedit {

SequenceIterator::HashVector
SequenceIterator::to_hash_vector(const uint64_t* nthash_hashes)
{
    HashVector hashes;
    for (unsigned i = 0; i < seeds.size(); i++) {
        uint64_t* seed_hashes = new uint64_t[nthash->get_hash_num_per_seed()];
        size_t i_begin = i * nthash->get_hash_num_per_seed();
        size_t i_end = i_begin + nthash->get_hash_num_per_seed();
        std::copy(nthash_hashes + i_begin, nthash_hashes + i_end, seed_hashes);
        hashes.push_back(seed_hashes);
    }
    return hashes;
}

void
SequenceIterator::next()
{
    nthash->roll();
}

void
SequenceIterator::previous()
{
    nthash->roll_back();
}

bool
SequenceIterator::has_next()
{
    return get_position() < seq.size() - nthash->get_k();
}

char
SequenceIterator::get_base(size_t position)
{
    return seq[position];
}

size_t
SequenceIterator::get_position()
{
    return nthash->get_pos() + nthash->get_k() - 1;
}

SequenceIterator::HashVector
SequenceIterator::get_hashes()
{
    return to_hash_vector(nthash->hashes());
}

const std::string&
SequenceIterator::get_sequence()
{
    return seq;
}

SequenceIterator::HashVector*
SequenceIterator::peek_hashes(unsigned window_size)
{
    HashVector* hashes = new HashVector[window_size];
    nthash->roll_back();
    auto peeked_hashes = nthash->peek_window(window_size);
    nthash->roll();
    for (unsigned i = 0; i < window_size; i++) {
        hashes[i] = to_hash_vector(peeked_hashes[i]);
    }
    return hashes;
}

unsigned
SequenceIterator::get_seed_length()
{
    return seeds[0].size();
}

void
SequenceIterator::update(size_t position, char value)
{
    seq[position] = value;
}

void
SequenceIterator::insert(size_t position, char value)
{}

void
SequenceIterator::remove(size_t position)
{}

}