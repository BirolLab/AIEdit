#include "AIEdit/utils.hpp"

std::vector<bool>
ai_edit::str_to_bool_vec(const std::string& str)
{
  std::vector<bool> vec;
  for (const auto& c : str) {
    vec.push_back(c == '1');
  }
  return vec;
}

std::string
ai_edit::bool_vec_to_str(const std::vector<bool>& vec)
{
  std::string str;
  str.reserve(vec.size());
  for (bool b : vec) {
    str.append(b ? "1" : "0");
  }
  return str;
}

bool
ai_edit::file_exists(const std::string& path)
{
  // TODO
  return false;
}