#include "aiedit/sequence_iterator.hpp"

namespace aiedit {

SequenceIterator::HashVector
SequenceIterator::to_hash_vector(const uint64_t* nthash_hashes)
{
    HashVector hashes;
    for (unsigned i = 0; i < seeds.size(); i++) {
        std::vector<uint64_t> seed_hashes;
        seed_hashes.reserve(nthash->get_hash_num_per_seed());
        size_t i_begin = i * nthash->get_hash_num_per_seed();
        size_t i_end = i_begin + nthash->get_hash_num_per_seed();
        seed_hashes.insert(seed_hashes.end(), nthash_hashes + i_begin, nthash_hashes + i_end);
        hashes.push_back(seed_hashes);
    }
    return hashes;
}

void
SequenceIterator::next(unsigned n)
{
    for (unsigned i = 0; i < n; i++) {
        nthash->roll();
    }
}

void
SequenceIterator::previous(unsigned n)
{
    for (unsigned i = 0; i < n; i++) {
        nthash->roll_back();
    }
}

bool
SequenceIterator::has_next()
{
    return get_position() < end - nthash->get_k();
}

char
SequenceIterator::get_base(size_t position)
{
    return seq[position];
}

std::string
SequenceIterator::get_bases(const std::vector<size_t>& positions)
{
    std::string sub_seq;
    sub_seq.reserve(positions.size());
    for (const auto& pos : positions) {
        sub_seq += seq[pos];
    }
    return sub_seq;
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

std::vector<SequenceIterator::HashVector>
SequenceIterator::peek_hashes(unsigned window_size)
{
    std::vector<HashVector> hashes;
    hashes.reserve(window_size);
    nthash->roll_back();
    for (const auto& peeked_hashes : nthash->peek_window(window_size)) {
        hashes.push_back(to_hash_vector(peeked_hashes.data()));
    }
    nthash->roll();
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

}