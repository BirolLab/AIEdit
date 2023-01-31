#ifndef AIEDIT_BLOOM_FILTER_MISMATCH_CORRECTOR_HPP
#define AIEDIT_BLOOM_FILTER_MISMATCH_CORRECTOR_HPP

#include <btllib/bloom_filter.hpp>
#include <nlohmann/json.hpp>

#include "aiedit/error_correction/error_corrector.hpp"
#include "aiedit/signature.hpp"

namespace aiedit {

class PatternDatabase
{
    friend class BloomFilterMismatchCorrector;
    PatternDatabase() = delete;

  private:
    PatternDatabase(unsigned pattern_length, const std::vector<std::string>& seeds)
      : pattern_length(pattern_length)
      , seeds(seeds)
    {
        initialize();
    }

    /**
     * Find the closest pattern mapped to the given signature
     * @param signature Hit/miss signature to query
     * @return Query result
     */
    const EditPattern& query(Signature& signature);

    /**
     * Get a JSON representation of the database
     * @return Database in JSON format
     */
    nlohmann::json to_json();

    const unsigned pattern_length;

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

class BloomFilterMismatchCorrector : public ErrorCorrector
{
  public:
    BloomFilterMismatchCorrector(unsigned pattern_length, const btllib::SeedBloomFilter& bf)
      : ErrorCorrector(pattern_length)
      , bf(bf)
      , db(pattern_length, bf.get_seeds())
    {}

    /**
     * Fix the mismatches in the current position of the sequence iterator
     * @param pattern Edit pattern for the current position
     * @return `true` if any edits were applied
     */
    bool fix(SequenceIterator& seq_iter) override;

  private:
    const btllib::SeedBloomFilter& bf;
    PatternDatabase db;

    /**
     * Get the positions in the sequence that need to be updated
     * @param base_position Position of the edit pattern in the sequence
     * @param pattern Edit pattern containing mismatches
     * @return Vector of positions containing mismatches
     */
    std::vector<size_t> get_mismatch_positions(const size_t base_position,
                                               const EditPattern& pattern);

    /**
     * Update the sequence with the new bases
     * @param positions Positions of the bases to be updated
     * @param new_bases New base values to replace the positions
     */
    void update_seq(SequenceIterator& seq_iter,
                    const std::vector<size_t>& positions,
                    const std::string& new_bases);

    /**
     * Peek the next signature and check for misses
     * @return `true` if the signature contains no misses
     */
    bool check_fixes(SequenceIterator& seq_iter);

    /**
     * Add local changes to list of fixes
     * @param positions List of updated positions in the sequence
     * @param reference Original content of the positions
     * @param updated Updated value in the positions
     */
    void add_edits(const std::vector<size_t>& positions,
                   const std::string& reference,
                   const std::string& updated);
};

}

#endif // AIEDIT_BLOOM_FILTER_MISMATCH_CORRECTOR_HPP