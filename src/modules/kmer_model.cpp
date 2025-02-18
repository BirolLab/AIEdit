#include "kmer_model.hpp"

#include <fstream>
#include <iterator>
#include <sstream>

namespace {

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

}

namespace aiedit {

KmerModel::KmerModel(const std::string& cbf_path,
                     const std::string& hist_path,
                     const std::string& seeds_path)
  : cbf(cbf_path)
  , probs(read_probs(hist_path))
  , seeds(read_seeds(seeds_path))
{}

unsigned KmerModel::get_num_hashes() const { return cbf.get_hash_num(); }

unsigned KmerModel::get_kmer_size() const { return seeds[0].size(); }

double KmerModel::score(const uint64_t* hashes) { return probs[cbf.contains(hashes)]; }

}
