#ifndef AIEDIT_PATTERN_DATABASE
#define AIEDIT_PATTERN_DATABASE

#include <nlohmann/json.hpp>
#include <vector>

#include "aiedit/edit_pattern.hpp"
#include "aiedit/pattern_detection/pattern_detector.hpp"
#include "aiedit/signature.hpp"

namespace aiedit {

class PatternDatabase : public PatternDetector
{
  public:
    PatternDatabase(unsigned pattern_length, const std::vector<std::string>& seeds)
      : PatternDetector(pattern_length)
      , seeds(seeds)
    {
        initialize();
    }

    /**
     * Find the closest pattern mapped to the given signature
     * @param signature Hit/miss signature to query
     * @return Query result
     */
    EditPattern get_pattern(Signature& signature) override;

    /**
     * Get a JSON representation of the database
     * @return Database in JSON format
     */
    nlohmann::json to_json();

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
    std::vector<std::pair<EditPattern, Signature>> entries;
};

}

#endif // AIEDIT_PATTERN_DATABASE