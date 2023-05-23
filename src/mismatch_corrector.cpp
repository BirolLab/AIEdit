#include "mismatch_corrector.hpp"

#include <bitset>
#include <fdeep/fdeep.hpp>

namespace {

using namespace aiedit;

const char ALPHABET[4] = {'A', 'C', 'G', 'T'};

/**
 * Generate all possible ACGT strings of length k recursively, except the
 * combinations that have the same content in the edit positions as the original
 * sequence.
 * @param prefix Prefix of the current combination.
 * @param k Size of the current combination.
 * @param original The original content of the edit positions in the sequence
 * concatenated as a single string.
 * @param combinations Container for the generated combinations.
 */
// NOLINTNEXTLINE (misc-no-recursion)
inline void get_combinations(const std::string& prefix,
                             unsigned k,
                             const std::string& original,
                             std::vector<std::string>& combinations)
{
    if (k == 0) {
        combinations.emplace_back(prefix);
        return;
    }
    for (const char c : ALPHABET) {
        if (original[original.size() - k] != c) {
            get_combinations(prefix + c, k - 1, original, combinations);
        }
    }
}

/**
 * Update the sequence with the new bases
 * @param positions Positions of the bases to be updated
 * @param new_bases New base values to replace the positions
 */
inline void update_seq(SequenceIterator& seq_iter,
                       const std::vector<size_t>& positions,
                       const std::string& new_bases)
{
    seq_iter.previous();
    for (unsigned i = 0; i < positions.size(); i++) {
        seq_iter.update(positions[i], new_bases[i]);
    }
    seq_iter.next();
}

/**
 * Get the positions in the sequence that need to be updated
 * @param base_position Position of the edit pattern in the sequence
 * @param pattern Edit pattern containing mismatches
 * @return Vector of positions containing mismatches
 */
inline std::vector<size_t> get_mismatch_positions(size_t base_position, const EditPattern& pattern)
{
    std::vector<size_t> positions;
    for (unsigned i = 0; i < pattern.get_length(); i++) {
        if (pattern.get(i) == Edit::Type::MISMATCH) {
            positions.push_back(base_position + i);
        }
    }
    return positions;
}

}  // namespace

namespace aiedit {

bool MismatchCorrector::check_fixes(SequenceIterator& seq_iter)
{
    auto signature_hashes = seq_iter.peek_hashes(seq_iter.get_seed_length());
    for (unsigned i = 0; i < seq_iter.get_seed_length(); i++) {
        auto hash_vector = signature_hashes[i];
        for (const auto& seed_hashes : hash_vector) {
            if (!bf.contains(seed_hashes)) {
                return false;
            }
        }
    }
    return true;
}

std::vector<Edit> MismatchCorrector::get_fixes(SequenceIterator& seq_iter,
                                               const EditPattern& pattern)
{
    auto mismatch_positions = get_mismatch_positions(seq_iter.get_position(), pattern);
    const std::string original = seq_iter.get_bases(mismatch_positions);
    std::string fixing_combination;
    std::vector<std::string> candidates;
    get_combinations("", original.size(), original, candidates);
    for (const auto& new_bases : candidates) {
        update_seq(seq_iter, mismatch_positions, new_bases);
        const bool fixed = check_fixes(seq_iter);
        if (fixed && fixing_combination.empty()) {
            fixing_combination = new_bases;
        } else if (fixed && !fixing_combination.empty()) {
            fixing_combination = std::string(mismatch_positions.size(), 'N');
            break;
        }
    }
    std::vector<Edit> edits;
    if (fixing_combination.empty()) {
        update_seq(seq_iter, mismatch_positions, original);
        return edits;
    }
    for (unsigned i = 0; i < mismatch_positions.size(); i++) {
        edits.emplace_back(mismatch_positions[i],
                           Edit::Type::MISMATCH,
                           original[i],
                           fixing_combination[i]);
    }
    update_seq(seq_iter, mismatch_positions, fixing_combination);
    return edits;
}

}  // namespace aiedit