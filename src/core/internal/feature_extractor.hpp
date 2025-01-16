#pragma once

#include <ATen/Tensor.h>
#include <btllib/counting_bloom_filter.hpp>
#include <vector>

namespace aiedit::internal {

class FeatureExtractor
{

  public:

    FeatureExtractor(const btllib::CountingBloomFilter8& cbf,
                     const std::vector<double>& probs,
                     const std::vector<std::string>& seeds,
                     unsigned max_indel);

    at::Tensor get_seed_encodings() const;

    at::Tensor extract(const std::string& seq, size_t pos_start, size_t pos_end);

  private:

    const btllib::CountingBloomFilter8& cbf;
    const std::vector<double>& probs;
    const std::vector<std::string>& seeds;
    const unsigned max_indel;
    const at::Tensor x_seeds;
};

}