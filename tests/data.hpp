#include <btllib/bloom_filter.hpp>

#include "edit_pattern.hpp"

struct MismatchTestData {
    std::string seq = "ATCGGATCGATC";
    std::string reference = "ATCGGCTCGATC";
    const size_t miss_position = 5;
    const size_t pattern_length = 3;
    const std::vector<std::string> seeds = {"11011", "11111"};
    const unsigned seed_length = seeds[0].size();
    const unsigned num_hashes_per_seed = 1;
    btllib::SeedBloomFilter* bf;

    MismatchTestData() { populate_bloom_filter(); }

    aiedit::EditPattern get_pattern()
    {
        aiedit::EditPattern pattern(pattern_length);
        pattern.set(0, aiedit::Edit::MISMATCH);
        return pattern;
    }

    std::vector<aiedit::Edit> get_true_edits()
    {
        std::vector<aiedit::Edit> edits;
        edits.push_back(aiedit::Edit(miss_position,
                                     aiedit::Edit::MISMATCH,
                                     std::string(1, seq[miss_position]),
                                     std::string(1, reference[miss_position])));
        return edits;
    }

  private:

    void populate_bloom_filter()
    {
        bf = new btllib::SeedBloomFilter(1024, seed_length, seeds, num_hashes_per_seed);
        bf->insert(reference.data(), reference.size());
    }
};
