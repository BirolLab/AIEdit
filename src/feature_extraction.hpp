#pragma once

#include <btllib/counting_bloom_filter.hpp>
#include <cctype>
#include <string>
#include <torch/torch.h>
#include <vector>

#include "gap_hash.hpp"

namespace aiedit {

torch::Tensor get_model_input(const std::string& seq,
                              unsigned start,
                              unsigned end,
                              const std::vector<std::string>& seeds,
                              unsigned max_indels,
                              const btllib::CountingBloomFilter8& cbf,
                              std::vector<float> error_probs)
{
    const auto k = seeds[0].size();
    const auto num_hashes = cbf.get_hash_num();
    auto x = torch::ones({end - start, (unsigned)seeds.size() + max_indels * 2 + 1});
    btllib::SeedNtHash nh(seq, seeds, num_hashes, k, start);
    while (nh.roll() && nh.get_pos() < end) {
        for (unsigned i = 0; i < seeds.size(); i++) {
            const auto hashes = nh.hashes() + i * num_hashes;
            x[nh.get_pos() - start][i + 1] = error_probs[cbf.contains(hashes)];
        }
    }
    aiedit::DeleteGapHash dh(seq, num_hashes, k, max_indels, start);
    char prev = 0;
    while (dh.roll() && dh.get_pos() < end) {
        char curr = std::toupper(seq[dh.get_pos() - start]);
        x[dh.get_pos() - start][0] = curr == prev ? 0.0 : 1.0;
        prev = curr;
        for (unsigned i = 0; i < max_indels; i++) {
            const auto prob = error_probs[cbf.contains(dh.hashes()[i])];
            x[dh.get_pos() - start][i + seeds.size() + 1] = prob;
        }
    }
    aiedit::InsertGapHash ih(seq, num_hashes, k, max_indels, start);
    while (ih.roll() && ih.get_pos() < end) {
        for (unsigned i = 0; i < max_indels; i++) {
            const auto prob = error_probs[cbf.contains(ih.hashes()[i])];
            x[ih.get_pos() - start][i + seeds.size() + max_indels + 1] = prob;
        }
    }
    return x;
};

torch::Tensor get_model_input_wrapper(const std::string& seq,
                                      unsigned start,
                                      unsigned end,
                                      const std::vector<std::string>& seeds,
                                      unsigned max_indels,
                                      uintptr_t cbf_ptr,
                                      std::vector<float> error_probs)
{
    auto* cbf = reinterpret_cast<btllib::CountingBloomFilter8*>(cbf_ptr);
    if (!cbf) {
        throw std::runtime_error("Invalid btllib::CountingBloomFilter pointer");
    }
    return get_model_input(seq, start, end, seeds, max_indels, *cbf, error_probs);
}

}  // namespace aiedit