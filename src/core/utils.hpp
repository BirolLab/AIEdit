#pragma once

#include <string>
#include <pybind11/stl.h>

namespace aiedit {

std::string apply_edits(const std::string_view seq, const pybind11::list& edits);

}