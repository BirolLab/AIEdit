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
 * Get the positions in the sequence that need to be updated
 * @param base_position Position of the edit pattern in the sequence
 * @param pattern Edit pattern containing mismatches
 * @return Vector of positions containing mismatches
 */
inline std::vector<size_t> get_mismatch_positions(size_t base_position, const Pattern& pattern)
{
    std::vector<size_t> positions;
    positions.reserve(pattern.get_length());
    for (unsigned i = 0; i < pattern.get_length(); i++) {
        if (pattern.get(i) == Edit::Type::MISMATCH) {
            positions.emplace_back(base_position + i);
        }
    }
    return positions;
}

/**
 * Build a subsequence of the original sequence
 * @param positions Positions of the bases
 * @return String built from the positions
 */
inline std::string get_bases(SequenceIterator& seq_iter, const std::vector<size_t>& positions)
{
    std::string bases;
    bases.resize(positions.size());
    for (unsigned i = 0; i < positions.size(); i++) {
        bases[i] = seq_iter.get_base(positions[i]);
    }
    return bases;
}

inline void update_seq(SequenceIterator& seq_iter,
                       const std::vector<size_t>& positions,
                       const std::string& new_bases)
{
    for (unsigned i = 0; i < positions.size(); i++) {
        seq_iter.update(positions[i], new_bases[i]);
    }
}

inline bool check_fixes(SequenceIterator& seq_iter,
                        const btllib::SeedBloomFilter& bf,
                        unsigned pattern_length,
                        const std::vector<size_t>& positions,
                        const std::string& fixes)
{
    auto bf_check = [&](const std::vector<uint64_t>& h) { return !bf.contains(h); };
    SequenceIterator seq_iter_copy(seq_iter);
    update_seq(seq_iter_copy, positions, fixes);
    unsigned signature_length = pattern_length + bf.get_k() - 1;
    while (signature_length-- > 0 && seq_iter_copy.has_next()) {
        seq_iter_copy.next();
        const auto& hashes = seq_iter_copy.get_hashes();
        if (std::any_of(hashes.begin(), hashes.end(), bf_check)) {
            return false;
        }
    }
    return true;
}

}  // namespace

namespace aiedit {

std::vector<Edit> MismatchCorrector::get_fixes(SequenceIterator& seq_iter, const Pattern& pattern)
{
    auto positions = get_mismatch_positions(seq_iter.get_position(), pattern);
    const std::string original = get_bases(seq_iter, positions);
    std::string fixes;
    std::vector<std::string> candidates;
    get_combinations("", original.size(), original, candidates);
    seq_iter.previous();
    for (const auto& new_bases : candidates) {
        const bool fixed = check_fixes(seq_iter, bf, pattern.get_length(), positions, new_bases);
        if (fixed && fixes.empty()) {
            fixes = new_bases;
        } else if (fixed && !fixes.empty()) {
            fixes = std::string(positions.size(), 'N');
            break;
        }
    }
    update_seq(seq_iter, positions, original);
    seq_iter.next();
    std::vector<Edit> edits;
    for (unsigned i = 0; i < fixes.size(); i++) {
        edits.emplace_back(positions[i], Edit::Type::MISMATCH, original[i], fixes[i]);
    }
    return edits;
}

}  // namespace aiedit