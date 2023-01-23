#ifndef AIEDIT_PATTERN_DATABASE
#define AIEDIT_PATTERN_DATABASE

#include <vector>

#include "edit_pattern.hpp"
#include "pattern_detection/pattern_detector.hpp"
#include "signature.hpp"

namespace aiedit {

class PatternDatabase : public PatternDetector
{
public:
  PatternDatabase(const std::vector<std::string>& seeds,
                  unsigned pattern_length,
                  unsigned signature_length)
    : pattern_length(pattern_length)
    , signature_length(signature_length)
  {
    initialize();
  }

  /**
   * Find the closest pattern mapped to the given signature
   * @param signature Hit/miss signature to query
   * @return Query result
  */
  EditPattern get_pattern(const Signature& signature) override;

private:
  /**
   * Initialize the pattern database
   */
  void initialize();

  const unsigned pattern_length;
  const unsigned signature_length;
  std::vector<std::pair<EditPattern, Signature>> entries;
};

};

#endif // AIEDIT_PATTERN_DATABASE