#include "aiedit.hpp"

#include "feature_extraction.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace {

std::vector<std::string> read_seeds(const std::string& seeds_path)
{
    std::ifstream file(seeds_path);
    return {std::istream_iterator<std::string>(file), std::istream_iterator<std::string>()};
}

bool next_hit(btllib::NtHash& nh,
              size_t end_kmer,
              const btllib::CountingBloomFilter8& cbf,
              const std::vector<double> probs,
              double threshold)
{
    while (nh.roll() && nh.get_pos() <= end_kmer) {
        if (probs[cbf.contains(nh.hashes())] < threshold) {
            return true;
        }
    }
    return false;
}

bool next_miss(btllib::NtHash& nh,
               size_t end_kmer,
               const btllib::CountingBloomFilter8& cbf,
               const std::vector<double> probs,
               double threshold)
{
    while (nh.roll() && nh.get_pos() <= end_kmer) {
        if (probs[cbf.contains(nh.hashes())] >= threshold) {
            return true;
        }
    }
    return false;
}

}  // namespace

namespace aiedit {

AIEdit::AIEdit(const std::string& cbf_path,
               const std::string& hist_path,
               const std::string& seeds_path,
               const std::string& model_path)
  : cprobs(hist_path, cbf_path, 0.5)
  , seeds(read_seeds(seeds_path))
  , model(model_path, seeds)
{}

size_t AIEdit::get_cbf_size() const { return cprobs.cbf.get_bytes(); }

unsigned AIEdit::get_max_indels() const { return model.model.attr("max_indels").toInt(); }

unsigned AIEdit::get_max_k() const { return model.model.attr("max_k").toInt(); }

unsigned AIEdit::get_k() const { return seeds[0].size(); }

unsigned AIEdit::get_num_seeds() const { return seeds.size(); }

void AIEdit::get_edits(const std::string& seq, size_t start, size_t end)
{
    const auto k = seeds[0].size();
    btllib::NtHash nh(seq, cprobs.cbf.get_hash_num(), k, start);
    if (!next_hit(nh, end - k, cprobs.cbf, cprobs.probs, 0.5)) {
        throw std::runtime_error("Could not find any robust k-mers");
    }
    while (next_miss(nh, end - k, cprobs.cbf, cprobs.probs, 0.5)) {
        const auto miss_pos = nh.get_pos();
        next_hit(nh, end - k, cprobs.cbf, cprobs.probs, 0.5);
    }
}

}  // namespace aiedit