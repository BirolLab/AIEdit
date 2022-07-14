#include "AIEdit/data_types.hpp"

std::string
ai_edit::Pattern::to_string() const
{
  std::string str;
  for (size_t i = 0; i < window_size; i++) {
    str.append(values[i] == PatternValue::MISMATCH ? "X" : "-");
  }
  return str;
}

void
ai_edit::Signature::push(ai_edit::Signature::SignatureValue* x)
{
  values.pop_front();
  values.push_back(x);
}

std::vector<std::string>
ai_edit::Signature::to_string_vec() const
{
  std::vector<std::string> str_vec;
  for (unsigned i = 0; i < frame_size; i++) {
    std::string row;
    for (unsigned j = 0; j < num_seeds; j++) {
      row.append(get(i, j) == SignatureValue::MISS ? "M" : "-");
    }
    str_vec.push_back(row);
  }
  return str_vec;
}

ai_edit::Signature
ai_edit::Signature::predict(const Pattern& pattern,
                            unsigned frame_size,
                            const std::vector<SpacedSeed>& seeds)
{
  Signature data(frame_size, seeds.size());
  bool is_care, is_mismatch;
  SignatureValue value;
  for (unsigned slide = 0; slide < frame_size; slide++) {
    for (unsigned i_seed = 0; i_seed < seeds.size(); i_seed++) {
      is_care = seeds[i_seed][seeds[i_seed].size() - 1 - slide] == '1';
      is_mismatch = pattern.get(slide) == Pattern::PatternValue::MISMATCH;
      if (is_care && is_mismatch) {
        value = SignatureValue::MISS;
      } else {
        value = SignatureValue::HIT;
      }
      data.set(slide, i_seed, value);
    }
  }
  return data;
}

unsigned
ai_edit::Pattern::get_num_edits() const
{
  unsigned num_edits = 0;
  for (size_t i = 0; i < window_size; i++) {
    if (values[i] != PatternValue::CLEAN) {
      ++num_edits;
    }
  }
  return num_edits;
}
