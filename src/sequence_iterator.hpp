#ifndef AIEDIT_SEQ_HPP
#define AIEDIT_SEQ_HPP

#include <deque>
#include <nthash/nthash.hpp>
#include <string>
#include <vector>

namespace aiedit {

class SequenceIterator
{
  public:

    SequenceIterator(std::string& seq, const std::vector<std::string>& seeds, unsigned num_hashes)
      : SequenceIterator(seq, seeds, num_hashes, 0, seq.size())
    {}

    SequenceIterator(std::string& seq,
                     const std::vector<std::string>& seeds,
                     unsigned num_hashes,
                     size_t begin,
                     size_t end)
      : seq(seq)
      , begin(begin)
      , end(end)
      , pos(begin + seeds[0].size() - 1)
      , current(seq[seeds[0].size()])
      , buffer(1, seq[seeds[0].size()])
      , hash_fn(seq, seeds, num_hashes, seeds[0].size(), begin)
      , num_seeds(seeds.size())
    {}

    SequenceIterator(SequenceIterator& seq_iter)
      : seq(seq_iter.seq)
      , begin(seq_iter.begin)
      , end(seq_iter.end)
      , pos(seq_iter.pos)
      , current(seq_iter.current)
      , buffer(seq_iter.buffer)
      , hash_fn(seq_iter.hash_fn)
      , num_seeds(seq_iter.num_seeds)
    {}

    bool next(unsigned n = 1);

    void substitute_last(char new_base);

    void delete_last();

    void insert_last(char new_base);

    char get_current() const;

    const std::vector<uint64_t> get_hashes(unsigned i) const;

    size_t get_position() const;

    unsigned get_seed_length() const;

    unsigned get_num_seeds() const;

  private:

    std::string_view seq;
    size_t begin, end, pos;
    char current;
    std::deque<char> buffer;
    nthash::BlindSeedNtHash hash_fn;
    const unsigned num_seeds;

    void consume();
};

}  // namespace aiedit

#endif  // AIEDIT_SEQ_HPP