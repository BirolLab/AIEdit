#pragma once

#include <btllib/counting_bloom_filter.hpp>
#include <string>
#include <vector>

namespace aiedit {

class CountProbabilities
{
  public:

    friend class AIEdit;

    CountProbabilities(const std::string& hist_path,
                       const std::string& cbf_path,
                       double max_hit_err);

    [[nodiscard]] bool is_hit(const uint64_t* hashes) const;
    [[nodiscard]] double get_prob(const uint64_t* hashes) const;

  private:

    btllib::CountingBloomFilter8 cbf;
    std::vector<double> probs;
    double max_hit_err;
};

}  // namespace aiedit