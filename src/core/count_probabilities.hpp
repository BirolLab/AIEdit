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
                       const std::string& cbf_path);

    double get_prob(const uint64_t* hashes) const;
    unsigned get_num_hashes() const;

  private:

    btllib::CountingBloomFilter8 cbf;
    std::vector<double> probs;
};

}