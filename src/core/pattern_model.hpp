#pragma once

#include <btllib/counting_bloom_filter.hpp>
#include <string>
#include <torch/script.h>
#include <vector>

namespace aiedit {

class PatternModel
{

  public:

    friend class AIEdit;

    PatternModel(const std::string& model_path, const std::vector<std::string>& seeds);

    [[nodiscard]] std::string get_pattern(const std::string& seq,
                                          size_t start,
                                          size_t end,
                                          const btllib::CountingBloomFilter8& cbf,
                                          const std::vector<double>& probs);

  private:

    torch::jit::script::Module model;
    std::vector<std::string> seeds;
    at::Tensor x_seeds;
};

}  // namespace aiedit