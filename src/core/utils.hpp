#pragma once

#include <string>
#include <vector>

#include "edit.hpp"

namespace aiedit::utils {

std::string apply_edits(const std::string& seq, const std::vector<Edit>& edits);

std::string pattern_to_string(const std::vector<Edit::Type>& pattern);

}