#include "sequence_iterator.hpp"

#include <iostream>

namespace {

using namespace aiedit;

template <typename T>
std::vector<T> operator+(const std::vector<T>& v1, const std::vector<T>& v2)
{
    std::vector<T> vr(std::begin(v1), std::end(v1));
    vr.insert(std::end(vr), std::begin(v2), std::end(v2));
    return vr;
}

inline void update_seed_hashes(const uint64_t* hash_array,
                               std::vector<std::vector<uint64_t>>& seed_hashes)
{
    const unsigned num_seeds = seed_hashes.size();
    const unsigned num_hashes_per_seed = seed_hashes[0].size();
    for (unsigned i = 0; i < num_seeds; i++) {
        for (unsigned j = 0; j < num_hashes_per_seed; j++) {
            seed_hashes[i][j] = hash_array[i * num_hashes_per_seed + j];
        }
    }
}

}  // namespace

namespace aiedit {

SequenceIterator::SequenceIterator(const std::string& seq,
                                   const std::vector<std::string>& seeds,
                                   unsigned num_hashes,
                                   size_t begin,
                                   size_t end)
  : seq(seq)
  , begin(begin)
  , end(end)
  , pos_next(begin + seeds[0].size())
  , previous(seq[begin + seeds[0].size() - 1])
  , current(seq[begin + seeds[0].size()])
  , buffer(1, seq[begin + seeds[0].size()])
  , hash_fn(seq.data(),
            std::vector<std::string>{std::string(seeds[0].size(), '1')} + seeds,
            num_hashes,
            seeds[0].size(),
            begin)
  , num_seeds(seeds.size())
{}

void SequenceIterator::consume()
{
    hash_fn.roll(buffer.front());
    previous = current;
    current = buffer.front();
    buffer.pop_front();
    if (buffer.empty() && pos_next < end) {
        buffer.push_back(seq[++pos_next]);
    }
}

bool SequenceIterator::has_next() { return !buffer.empty(); }

bool SequenceIterator::next(unsigned n)
{
    while (n-- > 0 && !buffer.empty()) {
        consume();
    }
    return has_next();
}

void SequenceIterator::substitute_last(char new_base)
{
    hash_fn.roll_back('A');
    hash_fn.roll(new_base);
    current = new_base;
}

void SequenceIterator::delete_last()
{
    if (!buffer.empty()) {
        hash_fn.roll_back('A');
        consume();
    }
}

void SequenceIterator::insert_last(char new_base)
{
    char backup = current;
    substitute_last(new_base);
    buffer.push_front(backup);
}

char SequenceIterator::get_previous() const { return previous; }

char SequenceIterator::get_current() const { return current; }

size_t SequenceIterator::get_position() const { return pos_next - 1; }

const std::vector<uint64_t> SequenceIterator::get_kmer_hashes() const
{
    std::vector<uint64_t> seed_hashes;
    seed_hashes.reserve(hash_fn.get_hash_num_per_seed());
    seed_hashes.insert(seed_hashes.end(),
                       hash_fn.hashes(),
                       hash_fn.hashes() + hash_fn.get_hash_num_per_seed());
    return seed_hashes;
}

const std::vector<uint64_t> SequenceIterator::get_seed_hashes(unsigned i) const
{
    std::vector<uint64_t> seed_hashes;
    seed_hashes.reserve(hash_fn.get_hash_num_per_seed());
    const unsigned i_begin = (i + 1) * hash_fn.get_hash_num_per_seed();
    const unsigned i_end = i_begin + hash_fn.get_hash_num_per_seed();
    seed_hashes.insert(seed_hashes.end(), hash_fn.hashes() + i_begin, hash_fn.hashes() + i_end);
    return seed_hashes;
}

unsigned SequenceIterator::get_k() const { return hash_fn.get_k(); }

unsigned SequenceIterator::get_num_seeds() const { return num_seeds; }

}  // namespace aiedit