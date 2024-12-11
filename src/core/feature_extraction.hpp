#pragma once

#include <ATen/Tensor.h>
#include <btllib/counting_bloom_filter.hpp>
#include <cctype>
#include <string>
#include <vector>

namespace aiedit {

at::Tensor positional_encoding(unsigned max_length, unsigned dim);

at::Tensor encode_seeds(const std::vector<std::string>& seeds, unsigned max_k);

at::Tensor get_model_input(const std::string& seq,
                           unsigned start,
                           unsigned end,
                           const std::vector<std::string>& seeds,
                           unsigned max_indels,
                           const btllib::CountingBloomFilter8& cbf,
                           std::vector<double> probs);

at::Tensor get_model_input_wrapper(const std::string& seq,
                                   unsigned start,
                                   unsigned end,
                                   const std::vector<std::string>& seeds,
                                   unsigned max_indels,
                                   uintptr_t cbf_ptr,
                                   std::vector<double> probs);

}  // namespace aiedit