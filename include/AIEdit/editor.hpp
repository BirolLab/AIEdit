#ifndef AIEDIT_EDITOR_HPP
#define AIEDIT_EDITOR_HPP

#include <string>

#include "AIEdit/data_types.hpp"

namespace ai_edit {

class Editor
{
private:
  std::string seq;
  std::vector<std::string> combinations;

  void update_combinations(std::string prefix, unsigned k);

public:
  Editor(std::string seq)
    : seq(seq)
  {}

  void apply(ai_edit::EditPattern& pattern,
             const size_t position,
             uint64_t*** signature_hashes);

  [[nodiscard]] std::string get_edited_seq() const { return seq; }
};

}

#endif // AIEDIT_EDITOR_HPP