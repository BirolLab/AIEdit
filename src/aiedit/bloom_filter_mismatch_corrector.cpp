#include <bitset>

#include "aiedit/error_correction/bloom_filter_mismatch_corrector.hpp"

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

void
PatternDatabase::initialize()
{
    for (unsigned p = 0; p < (1U << (pattern_length - 1)); p++) {
        std::string pattern_string = "1" + std::bitset<64>(p).to_string() + "1";
        std::reverse(pattern_string.begin(), pattern_string.end());
        EditPattern pattern(pattern_length);
        for (unsigned i = 0; i < pattern_length; i++) {
            bool is_mismatch = pattern_string[i] == '1';
            if (is_mismatch) {
                pattern.set(i, Edit::Type::MISMATCH);
            } else {
                pattern.set(i, Edit::Type::NONE);
            }
        }
        Signature signature = predict_signature(pattern);
        entries.push_back(std::make_pair(pattern, signature));
    }
}

Signature
PatternDatabase::predict_signature(EditPattern& pattern)
{
    Signature signature(seeds[0].size(), seeds.size());
    for (unsigned slide = 0; slide < seeds[0].size(); slide++) {
        for (unsigned i_seed = 0; i_seed < seeds.size(); i_seed++) {
            bool miss = false;
            auto seed = seeds[i_seed];
            unsigned overlap = std::min(slide + 1, pattern_length);
            for (unsigned pos = 0; pos < overlap; pos++) {
                bool is_error = pattern.get(pos) == Edit::Type::MISMATCH;
                bool is_care = seed[seed.size() - 1 - slide + pos] == '1';
                if (is_error && is_care) {
                    miss = true;
                }
            }
            signature.set(slide, i_seed, miss);
        }
    }
    return signature;
}

unsigned
PatternDatabase::get_distance(Signature& observed, Signature& from_database)
{
    unsigned distance = 0;
    for (unsigned i = 0; i < seeds[0].size(); i++) {
        for (unsigned j = 0; j < seeds.size(); j++) {
            bool t = observed.has_miss(i, j);
            bool d = from_database.has_miss(i, j);
            if (t != d) {
                for (auto& entry : entries) {
                    bool c = entry.second.has_miss(i, j);
                    if (t == c) {
                        ++distance;
                    }
                }
            }
        }
    }
    return distance;
}

const EditPattern&
PatternDatabase::query(Signature& signature)
{
    int i_result = -1;
    unsigned min_dist = std::numeric_limits<unsigned>::max();
    for (unsigned i = 0; i < entries.size(); i++) {
        unsigned dist = get_distance(signature, entries[i].second);
        if (i_result == -1 || dist <= min_dist) {
            i_result = i;
            min_dist = dist;
        }
    }
    return entries[i_result].first;
}

nlohmann::json
PatternDatabase::to_json()
{
    nlohmann::json db_json;
    for (auto& entry : entries) {
        auto pattern = entry.first.to_string();
        auto signature = entry.second.to_string_vector();
        db_json[pattern] = signature;
    }
    return db_json;
}

std::vector<size_t>
BloomFilterMismatchCorrector::get_mismatch_positions(const size_t base_position,
                                                     const EditPattern& pattern)
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
BloomFilterMismatchCorrector::update_seq(SequenceIterator& seq_iter,
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
BloomFilterMismatchCorrector::check_fixes(SequenceIterator& seq_iter)
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
BloomFilterMismatchCorrector::fix(SequenceIterator& seq_iter)
{
    Signature signature(seq_iter, bf);
    auto pattern = db.query(signature);
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
BloomFilterMismatchCorrector::add_edits(const std::vector<size_t>& positions,
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