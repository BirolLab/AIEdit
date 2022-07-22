#include "AIEdit/editor.hpp"

#include <btllib/nthash_lowlevel.hpp>

void
ai_edit::Editor::apply(ai_edit::EditPattern& pattern,
                       const size_t position,
                       ai_edit::Signature::Hashes hashes)
{
  std::vector<unsigned> edit_positions;
  for (const auto& offset : pattern.get_edit_positions()) {
    edit_positions.push_back(position + offset);
  }
  size_t num_edits = edit_positions.size();
  combinations.clear();
  update_combinations("", num_edits);
  unsigned found = 0;
  for (const auto& edits : combinations) {
    if (check(edit_positions, edits, hashes)) {
      ++found;
    }
  }
}

bool
ai_edit::Editor::check(std::vector<unsigned> edit_positions,
                       std::string edits,
                       ai_edit::Signature::Hashes hashes)
{
  for (size_t slide = 0; slide < signature_length; slide++) {
    for (size_t i_seed = 0; i_seed < filter.get_seeds().size(); i_seed++) {
      uint64_t* new_hashes = new uint64_t[filter.get_hash_num_per_seed()];
      btllib::sub_hash(hashes.get_forward_hash(slide, i_seed),
                       hashes.get_reverse_hash(slide, i_seed),
                       seq.data(),
                       edit_positions,
                       std::vector<unsigned char>(edits.begin(), edits.end()),
                       filter.get_seeds()[i_seed].size(),
                       filter.get_hash_num_per_seed(),
                       new_hashes);
      if (!filter.contains(new_hashes)) {
        return false;
      }
    }
  }
  return true;
}

void
ai_edit::Editor::update_combinations(std::string prefix, unsigned k)
{
  if (k == 0) {
    combinations.push_back(prefix);
  } else {
    for (const auto& c : "ACGT") {
      update_combinations(prefix + c, k - 1);
    }
  }
}