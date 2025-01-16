#pragma once

#include <string>
#include <vector>

namespace aiedit::internal {

std::vector<std::string> read_seeds(const std::string& path);

std::vector<double> read_probs(const std::string& path);

}