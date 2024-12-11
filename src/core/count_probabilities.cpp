#include "count_probabilities.hpp"

#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace aiedit {

CountProbabilities::CountProbabilities(const std::string& hist_path,
                                       const std::string& cbf_path,
                                       double max_hit_err)
  : cbf(cbf_path)
  , probs(256, 0.0)
  , max_hit_err(max_hit_err)
{
    std::ifstream file(hist_path);
    std::string line;
    unsigned long min_count = probs.size(), max_count = 0;
    std::getline(file, line);
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::istream_iterator<std::string> begin(ss);
        std::istream_iterator<std::string> end;
        std::vector<std::string> row(begin, end);
        const auto count = std::stoul(row[0]);
        if (count < probs.size()) {
            const auto p0 = std::stod(row[2]), p1 = std::stod(row[3]), p2 = std::stod(row[4]);
            probs[count] = p0 / (p0 + p1 + p2);
            min_count = std::min(min_count, count);
            max_count = std::max(max_count, count);
        }
    }
    for (unsigned long i = 0; i < min_count; i++) {
        probs[i] = 1.0;
    }
    file.close();
}

double CountProbabilities::get_prob(const uint64_t* hashes) const { return probs[cbf.contains(hashes)]; }

bool CountProbabilities::is_hit(const uint64_t* hashes) const { return get_prob(hashes) < max_hit_err; }

}  // namespace aiedit