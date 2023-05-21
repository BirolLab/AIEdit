#ifndef AIEDIT_MISMATCH_CORRECTOR_HPP
#define AIEDIT_MISMATCH_CORRECTOR_HPP

#include <btllib/bloom_filter.hpp>

#include "edit_pattern.hpp"
#include "sequence_iterator.hpp"

namespace aiedit {

class MismatchCorrector
{
  public:

    MismatchCorrector(unsigned pattern_length, const btllib::SeedBloomFilter& bf)
      : pattern_length(pattern_length)
      , bf(bf)
    {}

    /**
     * Fix the mismatches in the current position of the sequence iterator
     * @param seq_iter Sequence iterator pointing to the mismatch region
     * @return `true` if any edits were applied
     */
    bool fix(SequenceIterator& seq_iter, const EditPattern& pattern);

    /**
     * Clear the list of edits
     */
    void clear_edits() { edits.clear(); }

    /**
     * Get a list of applied edits
     * @return List of pairs containing the positions, type, and value of the
     * fixes
     */
    const std::vector<Edit>& get_edits() const { return edits; };

  private:

    const unsigned pattern_length;
    const btllib::SeedBloomFilter& bf;
    std::vector<Edit> edits;

    /**
     * Get the positions in the sequence that need to be updated
     * @param base_position Position of the edit pattern in the sequence
     * @param pattern Edit pattern containing mismatches
     * @return Vector of positions containing mismatches
     */
    static std::vector<size_t> get_mismatch_positions(size_t base_position,
                                                      const EditPattern& pattern);

    /**
     * Update the sequence with the new bases
     * @param positions Positions of the bases to be updated
     * @param new_bases New base values to replace the positions
     */
    static void update_seq(SequenceIterator& seq_iter,
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

}  // namespace aiedit

#endif  // AIEDIT_MISMATCH_CORRECTOR_HPP