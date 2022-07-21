#include "AIEdit/data_types.hpp"

std::string
ai_edit::EditPattern::to_string() const
{
  std::string str;
  for (size_t i = 0; i < window_size; i++) {
    str.append(values[i] == Value::MISMATCH ? "X" : "-");
  }
  return str;
}

void
ai_edit::Signature::push(ai_edit::Signature::Value* x)
{
  values.pop_front();
  values.push_back(x);
}

std::vector<std::string>
ai_edit::Signature::to_string_vec() const
{
  std::vector<std::string> str_vec;
  for (unsigned i = 0; i < length; i++) {
    std::string row;
    for (unsigned j = 0; j < num_seeds; j++) {
      row.append(get(i, j) == Value::MISS ? "M" : "-");
    }
    str_vec.push_back(row);
  }
  return str_vec;
}

ai_edit::Signature
ai_edit::Signature::predict(const EditPattern& pattern,
                            unsigned frame_size,
                            const std::vector<SpacedSeed>& seeds)
{
  Signature data(frame_size, seeds.size());
  for (unsigned slide = 0; slide < frame_size; slide++) {
    for (unsigned i_seed = 0; i_seed < seeds.size(); i_seed++) {
      bool miss = false;
      auto seed = seeds[i_seed];
      for (unsigned pos = 0; pos < std::min(slide, pattern.size()); pos++) {
        bool is_error = pattern.get(pos) == ai_edit::EditPattern::Value::MISMATCH;
        bool is_care = seed[seed.size() - 1 - slide + pos] == '1';
        if (is_error && is_care) {
          miss = true;
        }
      }
      if (miss) {
        data.set(slide, i_seed, ai_edit::Signature::Value::MISS);
      } else {
        data.set(slide, i_seed, ai_edit::Signature::Value::HIT);
      }
    }
  }
  return data;
}

unsigned
ai_edit::EditPattern::get_num_edits() const
{
  unsigned num_edits = 0;
  for (size_t i = 0; i < window_size; i++) {
    if (values[i] != Value::CLEAN) {
      ++num_edits;
    }
  }
  return num_edits;
}
