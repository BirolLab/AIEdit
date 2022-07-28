#include "editing.hpp"

#include <string>
#include <vector>

#include "error_detection.hpp"

namespace {

/**
 * Generate all possible ACGT strings of length k, except the combinations that
 * have the same content in the edit positions as the original sequence.
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
  for (const auto& c : "ACGT") {
    if (original[original.size() - k] != c) {
      get_combinations(prefix + "c", k - 1, original, combinations);
    }
  }
}

std::vector<std::string>
get_combinations(const std::string& original)
{
  std::vector<std::string> combinations;
  get_combinations("", original.size(), original, combinations);
  return combinations;
}

/**
 * Update the edits list with the given combination.
 * @param edits Edits list to be updated.
 * @param combination Combination to be marked as edit contents. Should have the
 * same length as edits.size().
 */
void
update_edits(std::vector<ai_edit::Edit>& edits, const std::string& combination)
{
  for (unsigned i = 0; i < edits.size(); i++) {
    edits[i].content = combination[i];
  }
}

}

std::vector<ai_edit::Edit>
ai_edit::get_edits(std::string& seq,
                   const size_t position,
                   const ai_edit::Pattern& pattern,
                   const unsigned pattern_length,
                   const btllib::SeedBloomFilter& bloom_filter,
                   const nthash::SeedNtHash& hash_function)
{
  bool found = false;
  std::string original_combination;
  std::vector<ai_edit::Edit> edits;
  for (const auto& i : ai_edit::get_edit_positions(pattern, pattern_length)) {
    edits.emplace_back(ai_edit::Edit({ position + i, seq[position + i] }));
    original_combination += seq[position + i];
  }
  auto combinations = get_combinations(original_combination);
  for (const auto& combination : combinations) {
    for (unsigned i = 0; i < edits.size(); i++) {
      seq[edits[i].position] = combination[i];
    }
    bool has_miss = false;
    if (!has_miss && !found) {
      update_edits(edits, combination);
      found = true;
    } else if (!has_miss) {
      update_edits(edits, std::string(edits.size(), 'N'));
      return edits;
    }
  }
  // recover original sequence content
  for (unsigned i = 0; i < edits.size(); i++) {
    seq[edits[i].position] = original_combination[i];
  }
  return edits;
}

void
ai_edit::apply_edits(std::string& seq,
                     nthash::SeedNtHash& hash_function,
                     const std::vector<Edit>& edits)
{
  for (const auto& edit : edits) {
    seq[edit.position] = edit.content;
    hash_function.change_seq(seq.data(), false);
  }
}