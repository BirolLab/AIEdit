#ifndef AIEDIT_BLOOM_FILTER_ERROR_DETECTOR_HPP
#define AIEDIT_BLOOM_FILTER_ERROR_DETECTOR_HPP

#include <btllib/bloom_filter.hpp>

#include "error_detection/error_detector.hpp"

namespace aiedit {

class BloomFilterErrorDetector : public ErrorDetector
{
public:
  BloomFilterErrorDetector(SequenceIterator& seq_iter,
                           const btllib::SeedBloomFilter& bf)
    : ErrorDetector(seq_iter)
    , bf(bf)
  {}

  /**
   * Check if the iterator can advance
   * @return `true` if the iterator has reached the end
   */
  bool has_error() override;

  /**
   * Advance the sequence iterator to the next miss position
   */
  void next_error() override;

private:
  /**
   * Check if the current position of the sequence iterator is a miss
   * @return `true` if the position is a miss
   */
  bool check_miss();

  const btllib::SeedBloomFilter& bf;
};

};

#endif // AIEDIT_BLOOM_FILTER_ERROR_DETECTOR_HPP