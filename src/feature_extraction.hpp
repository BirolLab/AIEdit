#pragma once

#include <btllib/counting_bloom_filter.hpp>
#include <torch/torch.h>

namespace aiedit {

torch::Tensor get_model_input(unsigned n)
{
    btllib::CountingBloomFilter8 cbf(n, 3);
    return torch::zeros({n});
}

};  // namespace aiedit