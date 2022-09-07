#include "editing.hpp"

#include <string>
#include <vector>

#include "error_detection.hpp"

namespace {

static const char ALPHABET[4] = { 'A', 'C', 'G', 'T' };

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
  for (size_t i = 0; i < 4; i++) {
    if (original[original.size() - k] != ALPHABET[i]) {
      get_combinations(prefix + ALPHABET[i], k - 1, original, combinations);
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
 * Make an edits list with the given combination.
 * @param base_position Position of the first erroneous base in the sequence.
 * @param edit_positions Positions of the edits in the pattern.
 * @param combination Combination to be marked as edit contents.
 */
std::vector<ai_edit::Edit>
create_edits_list(const size_t base_position,
                  std::vector<size_t> edit_positions,
                  const std::string& combination)
{
  std::vector<ai_edit::Edit> edits;
  for (unsigned i = 0; i < edit_positions.size(); i++) {
    size_t position = base_position + edit_positions[i];
    char content = combination[i];
    edits.emplace_back(ai_edit::Edit({ position, content }));
  }
  return edits;
}

/**
 * Apply the edits marked in the string to the sequence (in-place).
 * @param seq The sequence to be edited.
 * @param position The position of the first base to be edited.
 * @param edit_positions The positions of the bases to be edited relative to the
 * position argument.
 * @param edits String of edits.
 * @param hash_function SeedNtHash object to be updated with the new sequence.
 */
void
update_seq(std::string& seq,
           const size_t base_position,
           const std::vector<size_t> edit_positions,
           const std::string& edits,
           nthash::SeedNtHash& hash_function)
{
  hash_function.roll_back();
  for (unsigned i = 0; i < edits.size(); i++) {
    seq[base_position + edit_positions[i]] = edits[i];
  }
  hash_function.roll();
}

/**
 * Copy the positions marked as edits from the sequence.
 * @param seq Sequence string.
 * @param base_position The position of the first base to be edited.
 * @param edit_positions The positions of the bases to be edited relative to the
 * base_position.
 */
std::string
get_original_combination(const std::string& seq,
                         const size_t base_position,
                         const std::vector<size_t>& edit_positions)
{
  std::string original;
  for (const auto& i : edit_positions) {
    original += seq[base_position + i];
  }
  return original;
}

/**
 * Find the strings of edits that are supported by the Bloom filter.
 * @param seq The sequence string. Although this parameter is not const, the
 * contents of the sequence will not be changed after calling this function.
 * @param base_position The position of the first base to be edited.
 * @param edit_positions The positions of the bases to be edited relative to the
 * position argument.
 * @param bloom_filter Bloom filter for interrogation.
 * @param hash_function SeedNtHash object for generating hashes.
 * @param signature_length Length of the hit/miss signatures.
 */
std::vector<std::string>
find_supported_combinations(std::string& seq,
                            const size_t base_position,
                            const std::vector<size_t>& edit_positions,
                            const btllib::SeedBloomFilter& bloom_filter,
                            nthash::SeedNtHash& hash_function,
                            const unsigned signature_length)
{
  std::vector<std::string> combinations;
  unsigned num_seeds = bloom_filter.get_seeds().size();
  auto signature = ai_edit::create_signature(signature_length, num_seeds);
  auto original = get_original_combination(seq, base_position, edit_positions);
  for (const auto& combination : get_combinations(original)) {
    update_seq(seq, base_position, edit_positions, combination, hash_function);
    bool has_miss = ai_edit::update_signature(hash_function,
                                              bloom_filter,
                                              signature,
                                              signature_length);
    if (!has_miss) {
      combinations.emplace_back(combination);
    }
    if (combinations.size() > 1) {
      break;
    }
  }
  update_seq(seq, base_position, edit_positions, original, hash_function);
  return combinations;
}

}

std::vector<ai_edit::Edit>
ai_edit::get_edits(std::string& seq,
                   const size_t position,
                   const ai_edit::Pattern& pattern,
                   const unsigned pattern_length,
                   const btllib::SeedBloomFilter& bloom_filter,
                   nthash::SeedNtHash& hash_function,
                   const unsigned signature_length)
{
  auto edit_positions = ai_edit::get_edit_positions(pattern, pattern_length);
  auto combinations = find_supported_combinations(seq,
                                                  position,
                                                  edit_positions,
                                                  bloom_filter,
                                                  hash_function,
                                                  signature_length);
  if (combinations.size() == 0) {
    return std::vector<ai_edit::Edit>();
  } else if (combinations.size() == 1) {
    return create_edits_list(position, edit_positions, combinations[0]);
  } else {
    std::string mask = std::string(edit_positions.size(), 'N');
    return create_edits_list(position, edit_positions, mask);
  }
}

void
ai_edit::apply_edits(std::string& seq,
                     nthash::SeedNtHash& hash_function,
                     const std::vector<Edit>& edits)
{
  hash_function.roll_back();
  for (const auto& edit : edits) {
    seq[edit.position] = edit.content;
  }
  hash_function.roll();
}