#include "aiedit/error_correction/bf_mismatch_corrector.hpp"

namespace {

static const char ALPHABET[4] = { 'A', 'C', 'G', 'T' };

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
void
get_combinations(std::string prefix,
                 unsigned k,
                 const std::string& original,
                 std::vector<std::string>& combinations)
{
    if (k == 0) {
        combinations.emplace_back(prefix);
        return;
    }
    for (size_t i = 0; i < 4; i++) {
        if (original[original.size() - k] != ALPHABET[i]) {
            get_combinations(prefix + ALPHABET[i], k - 1, original, combinations);
        }
    }
}

};

namespace aiedit {

std::vector<size_t>
BloomFilterMismatchCorrector::get_mismatch_positions(const EditPattern& pattern)
{
    std::vector<size_t> positions;
    size_t base_position = seq_iter.get_position();
    for (size_t i = 0; i < pattern.get_length(); i++) {
        if (pattern.get(i) == Edit::Type::MISMATCH) {
            positions.push_back(base_position + i);
        }
    }
    return positions;
}

std::vector<std::string>
BloomFilterMismatchCorrector::get_candidates(const std::vector<size_t>& positions)
{
    std::string original;
    for (const auto& pos : positions) {
        original += seq_iter.get_base(pos);
    }
    std::vector<std::string> candidates;
    get_combinations("", original.size(), original, candidates);
    return candidates;
}

void
BloomFilterMismatchCorrector::update_seq(const std::vector<size_t>& positions, const std::string& new_bases)
{
    for (unsigned i = 0; i < positions.size(); i++) {
        seq_iter.update(positions[i], new_bases[i]);
    }
}

bool
BloomFilterMismatchCorrector::check_fixes()
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

bool
BloomFilterMismatchCorrector::fix(const EditPattern& pattern)
{
    auto base_position = seq_iter.get_position();
    auto mismatch_positions = get_mismatch_positions(pattern);
    std::string backup;
    for (const auto& pos : mismatch_positions) {
        backup += seq_iter.get_base(pos);
    }
    std::string fixing_combination;
    for (const auto& new_bases : get_candidates(mismatch_positions)) {
        update_seq(mismatch_positions, new_bases);
        bool fixed = check_fixes();
        if (fixed && fixing_combination.empty()) {
            fixing_combination = new_bases;
        } else if (!fixing_combination.empty()) {
            fixing_combination = std::string(mismatch_positions.size(), 'N');
            break;
        }
    }
    if (fixing_combination.empty()) {
        return false;
    }
    for (unsigned i = 0; i < mismatch_positions.size(); i++) {
        size_t pos = base_position + i;
        std::string reference = backup[i] + "";
        std::string updated = fixing_combination[i] + "";
        edits.push_back(Edit(pos, Edit::Type::MISMATCH, reference, updated));
    }
    update_seq(mismatch_positions, fixing_combination);
    return true;
}

}