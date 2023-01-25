#ifndef AIEDIT_BLOOM_FILTER_MISMATCH_CORRECTOR_HPP
#define AIEDIT_BLOOM_FILTER_MISMATCH_CORRECTOR_HPP

#include <btllib/bloom_filter.hpp>

#include "aiedit/error_correction/error_corrector.hpp"

namespace aiedit {

class BloomFilterMismatchCorrector : public ErrorCorrector
{
  public:
    BloomFilterMismatchCorrector(SequenceIterator& seq_iter, const btllib::SeedBloomFilter& bf)
      : ErrorCorrector(seq_iter)
      , bf(bf)
    {}

    /**
     * Fix the mismatches in the current position of the sequence iterator
     * @param pattern Edit pattern for the current position
     * @return `true` if any edits were applied
     */
    bool fix(const EditPattern& pattern) override;

  private:
    const btllib::SeedBloomFilter& bf;

    /**
     * Get the positions in the sequence that need to be updated
     * @param pattern Edit pattern for the sequence iterator's current state
     * @return Vector of positions containing mismatches
     */
    std::vector<size_t> get_mismatch_positions(const EditPattern& pattern);

    /**
     * Get new base combinations to test
     * @param positions List of mismatch positions in the sequence
     * @return Vector of strings representing new base values for the positions
     */
    std::vector<std::string> get_candidates(const std::vector<size_t>& positions);

    /**
     * Update the sequence with the new bases
     * @param positions Positions of the bases to be updated
     * @param new_bases New base values to replace the positions
     */
    void update_seq(const std::vector<size_t>& positions, const std::string& new_bases);

    /**
     * Peek the next signature and check for misses
     * @return `true` if the signature contains no misses
     */
    bool check_fixes();
};

}

#endif // AIEDIT_BLOOM_FILTER_MISMATCH_CORRECTOR_HPP