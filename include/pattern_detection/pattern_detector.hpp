#ifndef AIEDIT_PATTERN_DETECTOR_HPP
#define AIEDIT_PATTERN_DETECTOR_HPP

#include "edit_pattern.hpp"
#include "signature.hpp"

namespace aiedit {

class PatternDetector
{
public:
  /**
   * @param signature Hit/miss signature to query
   * @return Detected edit pattern
   */
  virtual EditPattern get_pattern(Signature& signature) = 0;
};

};

#endif // AIEDIT_PATTERN_DETECTOR_HPP