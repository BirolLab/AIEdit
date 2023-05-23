#include <btllib/bloom_filter.hpp>

#include "edit_pattern.hpp"

struct MismatchTestData {

    std::string seq = "ATCGGATCGATC";
    std::string reference = "ATCGGCTCGATC";
    const size_t miss_position = 5;
    const size_t pattern_length = 3;

    const std::vector<std::string> seeds = {"11011", "11111"};
    const unsigned seed_length = seeds[0].size();
    const unsigned bf_size = 1024, k = seeds[0].size(), num_hashes_per_seed = 1;
    btllib::SeedBloomFilter bf;

    MismatchTestData() : bf(bf_size, k, seeds, num_hashes_per_seed)
    {
        bf.insert(reference.data(), reference.size());
    }

    aiedit::EditPattern get_pattern() const
    {
        aiedit::EditPattern pattern(pattern_length);
        pattern.set(0, aiedit::Edit::MISMATCH);
        return pattern;
    }

    std::vector<aiedit::Edit> get_true_edits() const
    {
        std::vector<aiedit::Edit> edits;
        edits.emplace_back(miss_position,
                           aiedit::Edit::MISMATCH,
                           seq[miss_position],
                           reference[miss_position]);
        return edits;
    }
};
