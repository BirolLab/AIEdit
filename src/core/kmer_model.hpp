#pragma once

#include <btllib/counting_bloom_filter.hpp>
#include <string>
#include <vector>

namespace aiedit {

class KmerModel
{

  public:

    const std::vector<std::string> seeds;

    KmerModel(const std::string& cbf_path,
              const std::string& hist_path,
              const std::string& seeds_path,
              float hit_threshold = 0.5);

    unsigned get_num_hashes() const;

    unsigned get_kmer_size() const;

    double score(const uint64_t* hashes);

    bool is_hit(const uint64_t* hashes);

  private:

    const btllib::CountingBloomFilter8 cbf;
    const std::vector<double> probs;
    const float hit_threshold;
};

}