#ifndef PATTERN_DETECTOR_HPP
#define PATTERN_DETECTOR_HPP

#include <btllib/counting_bloom_filter.hpp>
#include <fdeep/fdeep.hpp>

#include "pattern.hpp"
#include "sequence_iterator.hpp"

namespace aiedit {

class PatternDetector
{

  public:

    PatternDetector(const btllib::CountingBloomFilter8& bf, const fdeep::model& model)
      : bf(bf)
      , model(model)
    {}

    /**
     * Get the edit pattern detected by the model
     * @param signature Input signature
     * @return Model's output as pattern object
     */
    Pattern get_pattern(SequenceIterator& seq_iter);

  private:

    const btllib::CountingBloomFilter8& bf;
    const fdeep::model& model;
};

}  // namespace aiedit

#endif  // PATTERN_DETECTOR_HPP