#include "feature_extractor.hpp"

#include "extensions/delete_gap_hash.hpp"
#include "extensions/insert_gap_hash.hpp"

#include <torch/torch.h>

namespace {

torch::Tensor encode_seeds(const std::vector<std::string>& seeds)
{
    auto x_seeds = torch::zeros({(unsigned)seeds[0].size(), (unsigned)seeds.size()});
    for (unsigned i = 0; i < seeds.size(); i++) {
        for (unsigned j = 0; j < seeds[i].size(); j++) {
            x_seeds[j][i] = static_cast<float>(seeds[i][j] - '0');
        }
    }
    return x_seeds;
}

}

namespace aiedit::internal {

FeatureExtractor::FeatureExtractor(const btllib::CountingBloomFilter8& cbf,
                                   const std::vector<double>& probs,
                                   const std::vector<std::string>& seeds,
                                   unsigned max_indel)
  : cbf(cbf)
  , probs(probs)
  , seeds(seeds)
  , max_indel(max_indel)
  , x_seeds(encode_seeds(seeds))
{}

at::Tensor FeatureExtractor::get_seed_encodings() const { return x_seeds; }

at::Tensor FeatureExtractor::extract(const std::string& seq, size_t start_pos, size_t end_pos)
{
    const auto k = seeds[0].size();
    const auto num_hashes = cbf.get_hash_num();
    unsigned start = start_pos, end = end_pos;
    auto x = torch::ones({end - start, (unsigned)seeds.size() + max_indel * 2 + 1});
    btllib::SeedNtHash nh(seq, seeds, num_hashes, k, start);
    while (nh.roll() && nh.get_pos() < end) {
        for (unsigned i = 0; i < seeds.size(); i++) {
            const auto hashes = nh.hashes() + i * num_hashes;
            x[nh.get_pos() - start][i + 1] = probs[cbf.contains(hashes)];
        }
    }
    DeleteGapHash dh(seq, num_hashes, k, max_indel, start);
    char prev = 0;
    while (dh.roll() && dh.get_pos() < end) {
        char curr = std::toupper(seq[dh.get_pos() - start]);
        x[dh.get_pos() - start][0] = curr == prev ? 0.0 : 1.0;
        prev = curr;
        for (unsigned i = 0; i < max_indel; i++) {
            const auto prob = probs[cbf.contains(dh.hashes()[i])];
            x[dh.get_pos() - start][i + seeds.size() + 1] = prob;
        }
    }
    InsertGapHash ih(seq, num_hashes, k, max_indel, start);
    while (ih.roll() && ih.get_pos() < end) {
        for (unsigned i = 0; i < max_indel; i++) {
            const auto prob = probs[cbf.contains(ih.hashes()[i])];
            x[ih.get_pos() - start][i + seeds.size() + max_indel + 1] = prob;
        }
    }
    return x;
}

}