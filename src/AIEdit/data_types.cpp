#include "AIEdit/data_types.hpp"

void
ai_edit::Signature::push(bool* x)
{
  values.pop_front();
  values.push_back(x);
}

std::vector<std::string>
ai_edit::Signature::to_string_vec() const
{
  std::vector<std::string> str_vec;
  str_vec.reserve(frame_size);
  for (const auto& arr : values) {
    std::string str;
    str.reserve(num_seeds);
    for (unsigned i = 0; i < num_seeds; i++) {
      str.append(arr[i] ? "1" : "0");
    }
    str_vec.push_back(str);
  }
  return str_vec;
}

ai_edit::Signature
ai_edit::Signature::predict(const Pattern& pattern,
                            unsigned frame_size,
                            const std::vector<SpacedSeed>& seeds)
{
  Signature data(frame_size, seeds.size());
  bool seed_value, pattern_value;
  for (unsigned slide = 0; slide < frame_size; slide++) {
    for (unsigned i_seed = 0; i_seed < seeds.size(); i_seed++) {
      seed_value = (seeds[i_seed][seeds[i_seed].size() - 1 - slide] == '1');
      pattern_value = pattern[slide];
      data.set(slide, i_seed, seed_value && pattern_value);
    }
  }
  return data;
}
