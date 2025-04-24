#pragma once

#include <pybind11/stl.h>
#include <string>

namespace aiedit {

std::string
apply_edits(const std::string_view seq, const pybind11::list& edits, float score_threshold);

}