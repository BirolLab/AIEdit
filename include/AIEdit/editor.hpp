#ifndef AIEDIT_EDITOR_HPP
#define AIEDIT_EDITOR_HPP

#include <btllib/bloom_filter.hpp>
#include <string>

#include "AIEdit/data_types.hpp"

namespace ai_edit {

class Editor
{
private:
  std::string seq;
  btllib::SeedBloomFilter& filter;
  const unsigned signature_length;
  std::vector<std::string> combinations;

  void update_combinations(std::string prefix, unsigned k);

  bool check(std::vector<unsigned> edit_positions,
             std::string edits,
             ai_edit::Signature::Hashes hashes);

public:
  Editor(std::string seq,
         btllib::SeedBloomFilter& filter,
         const unsigned signature_length)
    : seq(seq)
    , filter(filter)
    , signature_length(signature_length)
  {}

  void apply(ai_edit::EditPattern& pattern,
             const size_t position,
             ai_edit::Signature::Hashes hashes);

  [[nodiscard]] std::string get_edited_seq() const { return seq; }
};

}

#endif // AIEDIT_EDITOR_HPP