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

    SequenceIterator(const std::string& seq,
                     const std::vector<std::string>& seeds,
                     unsigned num_hashes,
                     size_t begin,
                     size_t end);

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
    size_t begin, end, pos_next;
    char current;
    std::deque<char> buffer;
    nthash::BlindSeedNtHash hash_fn;
    const unsigned num_seeds;

    void consume();
};

}  // namespace aiedit

#endif  // AIEDIT_SEQ_HPP