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

    PatternDetector(const std::vector<std::string>& patterns,
                    const btllib::CountingBloomFilter8& bf,
                    const fdeep::model& model)
      : pattern_length(patterns.front().size())
      , patterns(patterns)
      , bf(bf)
      , model(model)
    {}

    /**
     * Get the edit pattern detected by the model
     * @param signature Input signature
     * @return Model's output as pattern object
     */
    Pattern get_pattern(SequenceIterator& seq_iter);

  private:

    const unsigned pattern_length;
    const std::vector<std::string>& patterns;
    const btllib::CountingBloomFilter8& bf;
    const fdeep::model& model;

    /**
     * Prepare model input by updating the signature values
     * @param
     * @return Signature object containing model's input tensor
     */
    fdeep::tensor get_model_input(SequenceIterator& seq_iter);
};

}  // namespace aiedit

#endif  // PATTERN_DETECTOR_HPP