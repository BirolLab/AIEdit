#ifndef AIEDIT_BLOOM_FILTER_ERROR_DETECTOR_HPP
#define AIEDIT_BLOOM_FILTER_ERROR_DETECTOR_HPP

#include <btllib/bloom_filter.hpp>

#include "aiedit/error_detection/error_detector.hpp"

namespace aiedit {

class BloomFilterErrorDetector : public ErrorDetector
{
  public:

    BloomFilterErrorDetector(SequenceIterator& seq_iter, const btllib::SeedBloomFilter& bf)
      : ErrorDetector(seq_iter)
      , bf(bf)
    {}

    /**
     * Advance the sequence iterator to the next miss position
     * @return `false` if iteration has ended
     */
    bool next_error() override;

  private:

    const btllib::SeedBloomFilter& bf;

    /**
     * Check if the current position of the sequence iterator is a miss
     * @return `true` if the position is a miss
     */
    bool check_miss();
};

}  // namespace aiedit

#endif  // AIEDIT_BLOOM_FILTER_ERROR_DETECTOR_HPP