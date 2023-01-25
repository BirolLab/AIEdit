#ifndef AIEDIT_PATTERN_DETECTOR_HPP
#define AIEDIT_PATTERN_DETECTOR_HPP

#include "aiedit/edit_pattern.hpp"
#include "aiedit/signature.hpp"

namespace aiedit {

class PatternDetector
{
  protected:
    const unsigned pattern_length;

  public:
    PatternDetector(unsigned pattern_length)
      : pattern_length(pattern_length)
    {}

    virtual ~PatternDetector() = default;

    /**
     * @param signature Hit/miss signature to query
     * @return Detected edit pattern
     */
    virtual EditPattern get_pattern(Signature& signature) = 0;
};

}

#endif // AIEDIT_PATTERN_DETECTOR_HPP