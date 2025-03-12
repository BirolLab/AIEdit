#include "model_interface.hpp"

#include <stdexcept>

namespace {

constexpr auto BASES = "ACGT";

}

namespace aiedit {

ModelInterface::ModelInterface(const std::string_view seq,
                               size_t start,
                               size_t end,
                               const std::shared_ptr<KmerModel>& kmer_model)
  : prefix_kmer(seq.substr(start - 1, kmer_model->get_kmer_size()))
  , editor(seq, start + kmer_model->get_kmer_size() - 1, end + kmer_model->get_kmer_size() - 1)
  , kmer_model(kmer_model)
{}

Buffer2D ModelInterface::get_signature()
{
    Buffer2D signature(editor.get_size(), kmer_model->get_seeds().size() + 1);
    btllib::BlindSeedNtHash hash_fn(prefix_kmer.data(),
                                    kmer_model->get_seeds(),
                                    kmer_model->get_num_hashes(),
                                    kmer_model->get_kmer_size());
    char prev = prefix_kmer[prefix_kmer.size() - 1];
    for (const auto base : editor) {
        const auto pos = hash_fn.get_pos();
        signature.set(pos, 0, base == prev ? 1.0f : 0.0f);
        prev = base;
        hash_fn.roll(base);
        for (size_t seed = 0; seed < kmer_model->get_seeds().size(); seed++) {
            const uint64_t* hashes = hash_fn.hashes() + (seed * kmer_model->get_num_hashes());
            signature.set(pos, seed + 1, kmer_model->score(hashes));
        }
    }
    return signature;
}



}
