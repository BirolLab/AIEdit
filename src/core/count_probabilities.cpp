#include "count_probabilities.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace aiedit {

CountProbabilities::CountProbabilities(const std::string& hist_path, const std::string& cbf_path)
  : cbf(cbf_path)
  , probs(256, 0.0)
{
    std::ifstream hist_file(hist_path);
    if (!hist_file) {
        throw std::runtime_error("Unable to open seeds file: " + hist_path);
    }
    std::string line;
    unsigned long min_count = probs.size();
    std::getline(hist_file, line);
    while (std::getline(hist_file, line)) {
        std::stringstream ss(line);
        std::vector<std::string> row{std::istream_iterator<std::string>(ss),
                                     std::istream_iterator<std::string>()};
        const auto count = std::stoul(row[0]);
        if (count < probs.size()) {
            const auto p0 = std::stod(row[2]), p1 = std::stod(row[3]), p2 = std::stod(row[4]);
            probs[count] = p0 / (p0 + p1 + p2);
            min_count = std::min(min_count, count);
        }
    }
    for (unsigned long i = 0; i < min_count; i++) {
        probs[i] = 1.0;
    }
}

double CountProbabilities::get_prob(const uint64_t* hashes) const
{
    return probs[cbf.contains(hashes)];
}

}