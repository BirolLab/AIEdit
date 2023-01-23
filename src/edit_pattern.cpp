#include "edit_pattern.hpp"

namespace aiedit {

void
EditPattern::set(size_t index, Value value)
{
  values[index] = value;
}

EditPattern::Value
EditPattern::get(size_t index)
{
  return values[index];
}

std::string
EditPattern::to_string() const
{
  std::string s = "";
  for (size_t i = 0; i < length; i++) {
    s += (char)values[i];
  }
  return s;
}

std::vector<size_t>
EditPattern::get_edit_positions()
{
  std::vector<size_t> positions;
  for (size_t i = 0; i < length; i++) {
    if (values[i] != Value::NONE) {
      positions.push_back(i);
    }
  }
  return positions;
}

};