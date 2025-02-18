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
              const std::string& seeds_path);

    unsigned get_num_hashes() const;

    unsigned get_kmer_size() const;

    double score(const uint64_t* hashes);

  private:

    const btllib::CountingBloomFilter8 cbf;
    const std::vector<double> probs;
};

}