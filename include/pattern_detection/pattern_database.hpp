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
    : seeds(seeds)
    , pattern_length(pattern_length)
    , signature_length(signature_length)
  {
    initialize();
  }

  /**
   * Find the closest pattern mapped to the given signature
   * @param signature Hit/miss signature to query
   * @return Query result
   */
  EditPattern get_pattern(Signature& signature) override;

private:
  /**
   * Initialize the pattern database
   */
  void initialize();

  /**
   * Predict the hit/miss signature for the given edit pattern
   * @return Predicted signature
   */
  Signature predict_signature(EditPattern& pattern);

  /**
   * Compute the distance between two signatures
   * @param observed The observed hits and miss signature
   * @param from_database A postulated signature from the database
   * @return Distance between `observed` and `from_database`
   */
  unsigned get_distance(Signature& observed, Signature& from_database);

  const std::vector<std::string>& seeds;
  const unsigned pattern_length;
  const unsigned signature_length;
  std::vector<std::pair<EditPattern, Signature>> entries;
};

};

#endif // AIEDIT_PATTERN_DATABASE