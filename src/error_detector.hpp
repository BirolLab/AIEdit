#ifndef AIEDIT_ERROR_DETECTOR_HPP
#define AIEDIT_ERROR_DETECTOR_HPP

#include <btllib/bloom_filter.hpp>

#include "sequence_iterator.hpp"

namespace aiedit {

class ErrorDetector
{
  public:

    ErrorDetector(SequenceIterator& seq_iter, const btllib::SeedBloomFilter& bf)
      : seq_iter(seq_iter)
      , bf(bf)
    {}

    /**
     * Advance the sequence iterator to the next miss position
     * @return `false` if iteration has ended
     */
    bool find_next();

  private:

    SequenceIterator& seq_iter;
    const btllib::SeedBloomFilter& bf;

    /**
     * Check if the current position of the sequence iterator is a miss
     * @return `true` if the position is a miss
     */
    bool is_miss();
};

}  // namespace aiedit

#endif  // AIEDIT_ERROR_DETECTOR_HPP