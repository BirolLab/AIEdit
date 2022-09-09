#include <btllib/bloom_filter.hpp>
#include <nthash/nthash.hpp>

#include "editing.hpp"

struct MismatchTestData
{
  std::string seq = "ATCGGATCGATC";
  std::string reference = "ATCGGCTCGATC";
  const size_t miss_position = 5;
  const size_t pattern_length = 3;
  const std::vector<std::string> seeds = { "11011", "11111" };
  const unsigned seed_length = seeds[0].size();
  const unsigned num_hashes_per_seed = 1;
  btllib::SeedBloomFilter* bf;

  MismatchTestData() { populate_bloom_filter(); }

  ai_edit::Pattern get_pattern()
  {
    ai_edit::Pattern pattern = ai_edit::create_pattern(pattern_length);
    pattern[0] = ai_edit::PatternValue::MISMATCH;
    return pattern;
  }

  std::vector<ai_edit::Edit> get_true_edits()
  {
    std::vector<ai_edit::Edit> edits;
    edits.push_back({ miss_position, reference[miss_position] });
    return edits;
  }

private:
  void populate_bloom_filter()
  {
    bf = new btllib::SeedBloomFilter(1024,
                                     seed_length,
                                     seeds,
                                     num_hashes_per_seed);
    bf->insert(reference.data(), reference.size());
  }
};
