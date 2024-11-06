#pragma once

#include <btllib/counting_bloom_filter.hpp>

#include "sequence_iterator.hpp"

class ErrorDetector
{
  public:

    ErrorDetector(SequenceIterator& seq_iter, const btllib::CountingBloomFilter8& bf)
      : seq_iter(seq_iter)
      , bf(bf)
    {}

    /**
     * Advance the sequence iterator to the next miss position
     * @return `false` if iteration has ended
     */
    bool next()
    {
        bool has_miss = false;
        while (!has_miss && seq_iter.next()) {
            has_miss = bf.contains(seq_iter.get_kmer_hashes()) == 0;
        }
        return has_miss;
    }

  private:

    SequenceIterator& seq_iter;
    const btllib::CountingBloomFilter8& bf;
};
