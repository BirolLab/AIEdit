#pragma once

#include <btllib/counting_bloom_filter.hpp>
#include <memory>
#include <string>
#include <torch/script.h>
#include <vector>

#include "edit.hpp"

namespace aiedit {

class PatternModel
{

  public:

    PatternModel(const std::string& model_path, const std::string& seeds_path);

    std::vector<Edit::Type> get_pattern(const std::string& seq,
                                        size_t start,
                                        size_t end,
                                        const btllib::CountingBloomFilter8& cbf,
                                        const std::vector<double>& probs);

    size_t get_k() const;
    int64_t get_max_indels() const;
    int64_t get_max_k() const;
    int64_t get_num_seeds() const;

  private:

    torch::jit::Module model;
    std::vector<std::string> seeds;
    int64_t num_seeds, max_k, max_indels;
    at::Tensor x_seeds;
};

}