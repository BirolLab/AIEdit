#pragma once

#include <btllib/counting_bloom_filter.hpp>
#include <cctype>
#include <torch/torch.h>

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
    auto x = torch::ones({(unsigned)seeds.size() + max_indels + 1, end - start});
    btllib::SeedNtHash nh(seq, seeds, num_hashes, k, start);
    while (nh.roll() && nh.get_pos() < end) {
        for (unsigned i = 0; i < seeds.size(); i++) {
            const auto hashes = nh.hashes() + i * num_hashes;
            x[i + 1][nh.get_pos() - start] = error_probs[cbf.contains(hashes)];
        }
    }
    aiedit::GapHash gh(seq, num_hashes, k, max_indels, start);
    char prev = 0;
    while (gh.roll() && gh.get_pos() < end - k / 2) {
        char curr = std::toupper(seq[gh.get_pos() - start + k / 2]);
        x[0][gh.get_pos() - start + k / 2] = curr == prev ? 0.0 : 1.0;
        prev = curr;
        for (unsigned i = 0; i < max_indels; i++) {
            const auto prob = error_probs[cbf.contains(gh.hashes()[i])];
            x[i + seeds.size() + 1][gh.get_pos() - start + k / 2] = prob;
        }
    }
    return x;
};

}  // namespace aiedit