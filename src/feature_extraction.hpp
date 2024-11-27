#pragma once

#include <btllib/counting_bloom_filter.hpp>
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
    auto x = torch::empty({(unsigned)seeds.size() + max_indels, end - start});
    btllib::SeedNtHash nh(seq, seeds, num_hashes, k, start);
    while (nh.roll() && nh.get_pos() < end) {
        for (unsigned i = 0; i < seeds.size(); i++) {
            const auto hashes = nh.hashes() + i * num_hashes;
            x[i][nh.get_pos() - start] = error_probs[cbf.contains(hashes)];
        }
    }
    aiedit::GapHash gh(seq, num_hashes, k, max_indels, start);
    while (gh.roll() && gh.get_pos() < end) {
        for (unsigned i = 0; i < max_indels; i++) {
            const auto prob = error_probs[cbf.contains(gh.hashes()[i])];
            x[i + seeds.size()][gh.get_pos() - start - k / 2] = prob;
        }
    }
    const auto i_slice = torch::indexing::Slice(seeds.size(), torch::indexing::None);
    const auto j_slice = torch::indexing::Slice(end-start-k / 2, torch::indexing::None);
    x.index_put_({i_slice, j_slice}, 1.0);
    return x;
}

};  // namespace aiedit