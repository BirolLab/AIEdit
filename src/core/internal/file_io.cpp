#include "file_io.hpp"

#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace aiedit::internal {

std::vector<std::string> read_seeds(const std::string& path)
{
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Unable to open file: " + path);
    }
    std::vector<std::string> seeds;
    std::string line;
    while (std::getline(file, line)) {
        seeds.push_back(line);
    }
    return seeds;
}

std::vector<double> read_probs(const std::string& path)
{
    std::vector<double> probs(256, 0.0);
    std::ifstream probs_file(path);
    if (!probs_file) {
        throw std::runtime_error("Unable to open file: " + path);
    }
    std::string line;
    unsigned long min_count = probs.size();
    std::getline(probs_file, line);
    while (std::getline(probs_file, line)) {
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
    return probs;
}

}