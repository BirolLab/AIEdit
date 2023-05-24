#ifndef AIEDIT_ERROR_DETECTOR_HPP
#define AIEDIT_ERROR_DETECTOR_HPP

#include <btllib/counting_bloom_filter.hpp>

#include "sequence_iterator.hpp"

namespace aiedit {

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
    bool next();

  private:

    SequenceIterator& seq_iter;
    const btllib::CountingBloomFilter8& bf;

    /**
     * Check if the current position of the sequence iterator is a miss
     * @return `true` if the position is a miss
     */
    bool is_miss();
};

}  // namespace aiedit

#endif  // AIEDIT_ERROR_DETECTOR_HPP