#include "data_types.hpp"

ai_edit::Signature
ai_edit::create_signature(const size_t length, const unsigned num_seeds)
{
  auto signature = new SignatureValue*[length];
  for (size_t i = 0; i < length; i++) {
    signature[i] = new SignatureValue[num_seeds];
    for (size_t j = 0; j < num_seeds; j++) {
      signature[i][j] = SignatureValue::HIT;
    }
  }
  return signature;
}

std::vector<std::string>
ai_edit::signature_to_string_vec(const ai_edit::Signature& signature,
                                 const unsigned signature_length,
                                 const unsigned num_seeds)
{
  std::vector<std::string> rows;
  for (size_t i = 0; i < signature_length; i++) {
    std::string row;
    for (size_t j = 0; j < num_seeds; j++) {
      row.append(signature[i][j] == SignatureValue::MISS ? "X" : "-");
    }
    rows.emplace_back(row);
  }
  return rows;
}

std::string
ai_edit::pattern_to_string(const ai_edit::Pattern& pattern,
                           const unsigned pattern_length)
{
  std::string pattern_string;
  for (size_t i = 0; i < pattern_length; i++) {
    pattern_string.append(pattern[i] == PatternValue::MISMATCH ? "M" : "-");
  }
  return pattern_string;
}

std::vector<size_t>
ai_edit::get_edit_positions(const ai_edit::Pattern& pattern,
                            const unsigned pattern_length)
{
  std::vector<size_t> positions;
  for (size_t i = 0; i < pattern_length; i++) {
    if (pattern[i] != PatternValue::CLEAN) {
      positions.emplace_back(i);
    }
  }
  return positions;
}