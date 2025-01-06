#pragma once

#include <string>
#include <vector>

#include "edit.hpp"

namespace aiedit {

std::string apply_edits(const std::string& seq, const std::vector<Edit>& edits);

}