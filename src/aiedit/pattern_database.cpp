#include "aiedit/pattern_detection/pattern_database.hpp"

#include <algorithm>
#include <bitset>

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
        auto signature = predict_signature(pattern);
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

EditPattern
PatternDatabase::get_pattern(Signature& signature)
{
    EditPattern* result = nullptr;
    unsigned min_dist = std::numeric_limits<unsigned>::max();
    for (auto& entry : entries) {
        unsigned dist = get_distance(signature, entry.second);
        if (result == nullptr || dist <= min_dist) {
            result = &entry.first;
            min_dist = dist;
        }
    }
    return *result;
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

}