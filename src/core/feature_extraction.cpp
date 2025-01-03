#include "feature_extraction.hpp"

#include "extensions/delete_gap_hash.hpp"
#include "extensions/insert_gap_hash.hpp"

#include <torch/torch.h>

namespace aiedit {

torch::Tensor positional_encoding(unsigned max_length, unsigned dim)
{
    auto position = torch::arange(max_length).unsqueeze(1).to(torch::kFloat);
    auto div_term = torch::arange(0, dim, 2, torch::kFloat);
    div_term = torch::exp(div_term * (-std::log(10000.0) / dim));
    auto pos_enc = torch::zeros({max_length, dim});
    auto sin_values = torch::sin(position * div_term);
    auto even_indices = torch::arange(0, dim, 2, torch::kLong);
    pos_enc.index_put_({torch::indexing::Slice(), even_indices}, sin_values);
    auto cos_values = torch::cos(position * div_term);
    auto odd_indices = torch::arange(1, dim, 2, torch::kLong);
    if (dim % 2 != 0) {
        auto end_idx = torch::indexing::Slice(torch::indexing::None, odd_indices.size(0));
        cos_values = cos_values.index({torch::indexing::Slice(), end_idx});
    }
    pos_enc.index_put_({torch::indexing::Slice(), odd_indices}, cos_values);

    return pos_enc;
}

torch::Tensor encode_seeds(const std::vector<std::string>& seeds, unsigned max_k)
{
    auto x_seeds = torch::zeros({(unsigned)seeds.size(), max_k});
    for (unsigned i = 0; i < seeds.size(); i++) {
        for (unsigned j = 0; j < seeds[i].size(); j++) {
            x_seeds[i][j] = static_cast<float>(seeds[i][j] - '0');
        }
    }
    return x_seeds;
}

torch::Tensor get_model_input(const std::string& seq,
                              unsigned start,
                              unsigned end,
                              const std::vector<std::string>& seeds,
                              unsigned max_indels,
                              const btllib::CountingBloomFilter8& cbf,
                              std::vector<double> probs)
{
    const auto k = seeds[0].size();
    const auto num_hashes = cbf.get_hash_num();
    auto x = torch::ones({end - start, (unsigned)seeds.size() + max_indels * 2 + 1});
    btllib::SeedNtHash nh(seq, seeds, num_hashes, k, start);
    while (nh.roll() && nh.get_pos() < end) {
        for (unsigned i = 0; i < seeds.size(); i++) {
            const auto hashes = nh.hashes() + i * num_hashes;
            x[nh.get_pos() - start][i + 1] = probs[cbf.contains(hashes)];
        }
    }
    DeleteGapHash dh(seq, num_hashes, k, max_indels, start);
    char prev = 0;
    while (dh.roll() && dh.get_pos() < end) {
        char curr = std::toupper(seq[dh.get_pos() - start]);
        x[dh.get_pos() - start][0] = curr == prev ? 0.0 : 1.0;
        prev = curr;
        for (unsigned i = 0; i < max_indels; i++) {
            const auto prob = probs[cbf.contains(dh.hashes()[i])];
            x[dh.get_pos() - start][i + seeds.size() + 1] = prob;
        }
    }
    InsertGapHash ih(seq, num_hashes, k, max_indels, start);
    while (ih.roll() && ih.get_pos() < end) {
        for (unsigned i = 0; i < max_indels; i++) {
            const auto prob = probs[cbf.contains(ih.hashes()[i])];
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
                                      std::vector<double> probs)
{
    auto* cbf = reinterpret_cast<btllib::CountingBloomFilter8*>(cbf_ptr);
    if (!cbf) {
        throw std::runtime_error("Invalid btllib::CountingBloomFilter pointer");
    }
    return get_model_input(seq, start, end, seeds, max_indels, *cbf, probs);
}

}