#include <bitset>
#include <fdeep/fdeep.hpp>

#include "aiedit/error_correction/mismatch_corrector.hpp"

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

fdeep::tensor
MismatchCorrector::get_model_input(SequenceIterator& seq_iter)
{
    unsigned signature_length = pattern_length + bf.get_seeds()[0].size() - 1;
    Signature signature(signature_length, bf.get_seeds().size());
    auto hashes = seq_iter.peek_hashes(signature.get_length());
    for (size_t i = 0; i < signature.get_length(); i++) {
        for (size_t j = 0; j < signature.get_num_seeds(); j++) {
            signature.set(i, j, !bf.contains(hashes[i][j]));
        }
    }
    return signature.get_tensor();
}

EditPattern
MismatchCorrector::get_pattern(const fdeep::tensor& model_input)
{
    auto model_output = model.predict({ model_input });
    EditPattern pattern(pattern_length);
    for (size_t i = 0; i < pattern.get_length(); i++) {
        if (model_output[0].get(fdeep::tensor_pos(i)) >= 0.5) {
            pattern.set(i, Edit::MISMATCH);
        } else {
            pattern.set(i, Edit::NONE);
        }
    }
    return pattern;
}

std::vector<size_t>
MismatchCorrector::get_mismatch_positions(const size_t base_position, const EditPattern& pattern)
{
    std::vector<size_t> positions;
    for (size_t i = 0; i < pattern.get_length(); i++) {
        if (pattern.get(i) == Edit::Type::MISMATCH) {
            positions.push_back(base_position + i);
        }
    }
    return positions;
}

void
MismatchCorrector::update_seq(SequenceIterator& seq_iter,
                              const std::vector<size_t>& positions,
                              const std::string& new_bases)
{
    seq_iter.previous();
    for (unsigned i = 0; i < positions.size(); i++) {
        seq_iter.update(positions[i], new_bases[i]);
    }
    seq_iter.next();
}

bool
MismatchCorrector::check_fixes(SequenceIterator& seq_iter)
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
MismatchCorrector::fix(SequenceIterator& seq_iter)
{
    auto model_input = get_model_input(seq_iter);
    auto pattern = get_pattern(model_input);
    auto mismatch_positions = get_mismatch_positions(seq_iter.get_position(), pattern);
    std::string original = seq_iter.get_bases(mismatch_positions);
    std::string fixing_combination;
    std::vector<std::string> candidates;
    get_combinations("", original.size(), original, candidates);
    for (const auto& new_bases : candidates) {
        update_seq(seq_iter, mismatch_positions, new_bases);
        bool fixed = check_fixes(seq_iter);
        if (fixed && fixing_combination.empty()) {
            fixing_combination = new_bases;
        } else if (fixed && !fixing_combination.empty()) {
            fixing_combination = std::string(mismatch_positions.size(), 'N');
            break;
        }
    }
    if (fixing_combination.empty()) {
        update_seq(seq_iter, mismatch_positions, original);
        return false;
    }
    add_edits(mismatch_positions, original, fixing_combination);
    update_seq(seq_iter, mismatch_positions, fixing_combination);
    return true;
}

void
MismatchCorrector::add_edits(const std::vector<size_t>& positions,
                             const std::string& reference,
                             const std::string& updated)
{
    for (unsigned i = 0; i < positions.size(); i++) {
        size_t pos = positions[i];
        std::string ref(1, reference[i]);
        std::string alt(1, updated[i]);
        edits.push_back(Edit(pos, Edit::Type::MISMATCH, ref, alt));
    }
}

}