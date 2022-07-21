#include "AIEdit/editor.hpp"

void
ai_edit::Editor::apply(ai_edit::EditPattern& pattern,
                       const size_t position,
                       uint64_t*** signature_hashes)
{
  auto edit_positions = pattern.get_edit_positions();
  size_t num_edits = edit_positions.size();
  combinations.clear();
  update_combinations("", num_edits);
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