#pragma once

#include <btllib/nthash.hpp>
#include <deque>
#include <iostream>
#include <string>
#include <vector>

template <typename T>
std::vector<T> operator+(const std::vector<T>& v1, const std::vector<T>& v2)
{
    std::vector<T> vr(std::begin(v1), std::end(v1));
    vr.insert(std::end(vr), std::begin(v2), std::end(v2));
    return vr;
}

class SequenceIterator
{
  public:

    SequenceIterator(const std::string& seq,
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

    SequenceIterator(const std::string& seq,
                     const std::vector<std::string>& seeds,
                     unsigned num_hashes)
      : SequenceIterator(seq, seeds, num_hashes, 0, seq.size())
    {}

    SequenceIterator(SequenceIterator& seq_iter)
      : seq(seq_iter.seq)
      , begin(seq_iter.begin)
      , end(seq_iter.end)
      , pos_next(seq_iter.pos_next)
      , previous(seq_iter.previous)
      , current(seq_iter.current)
      , buffer(seq_iter.buffer)
      , hash_fn(seq_iter.hash_fn)
      , num_seeds(seq_iter.num_seeds)
    {}

    bool has_next() { return !buffer.empty(); }

    bool next(unsigned n = 1)
    {
        while (n-- > 0 && !buffer.empty()) {
            consume();
        }
        return has_next();
    }

    void substitute_last(char new_base)
    {
        hash_fn.roll_back('A');
        hash_fn.roll(new_base);
        current = new_base;
    }

    void delete_last()
    {
        if (!buffer.empty()) {
            hash_fn.roll_back('A');
            consume();
        }
    }

    void insert_last(char new_base)
    {
        char backup = current;
        substitute_last(new_base);
        buffer.push_front(backup);
    }

    char get_previous() const { return previous; }

    char get_current() const { return current; }

    const std::vector<uint64_t> get_kmer_hashes() const
    {
        std::vector<uint64_t> seed_hashes;
        seed_hashes.reserve(hash_fn.get_hash_num_per_seed());
        seed_hashes.insert(seed_hashes.end(),
                           hash_fn.hashes(),
                           hash_fn.hashes() + hash_fn.get_hash_num_per_seed());
        return seed_hashes;
    }

    const std::vector<uint64_t> get_seed_hashes(unsigned i) const
    {
        std::vector<uint64_t> seed_hashes;
        seed_hashes.reserve(hash_fn.get_hash_num_per_seed());
        const unsigned i_begin = (i + 1) * hash_fn.get_hash_num_per_seed();
        const unsigned i_end = i_begin + hash_fn.get_hash_num_per_seed();
        seed_hashes.insert(seed_hashes.end(), hash_fn.hashes() + i_begin, hash_fn.hashes() + i_end);
        return seed_hashes;
    }

    size_t get_position() const { return pos_next - 1; }

    unsigned get_k() const { return hash_fn.get_k(); }

    unsigned get_num_seeds() const { return num_seeds; }

  private:

    std::string_view seq;
    size_t begin, end, pos_next;
    char previous, current;
    std::deque<char> buffer;
    btllib::BlindSeedNtHash hash_fn;
    const unsigned num_seeds;

    void consume()
    {
        hash_fn.roll(buffer.front());
        previous = current;
        current = buffer.front();
        buffer.pop_front();
        if (buffer.empty() && pos_next < end) {
            buffer.push_back(seq[++pos_next]);
        }
    }

    void update_seed_hashes(const uint64_t* hash_array,
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
};
