#include "model_interface.hpp"

#include <stdexcept>

namespace {

constexpr auto BASES = "ACGT";

inline void fill_repeats_row(aiedit::Buffer2D& signature,
                             const std::string& prefix_kmer,
                             aiedit::Editor& editor)
{
    char prev = prefix_kmer[prefix_kmer.size() - 1];
    size_t pos = 0;
    for (const auto base : editor) {
        signature.set(pos++, 0, base == prev ? 1.0f : 0.0f);
        prev = base;
    }
}

inline void fill_seed_row(aiedit::Buffer2D& signature,
                          unsigned i_seed,
                          const std::string& prefix_kmer,
                          aiedit::Editor& editor,
                          const std::shared_ptr<aiedit::KmerModel>& kmer_model,
                          unsigned max_ins)
{
    btllib::BlindSeedNtHash hash_fn(prefix_kmer.data(),
                                    {kmer_model->get_seeds()[i_seed]},
                                    kmer_model->get_num_hashes(),
                                    kmer_model->get_kmer_size());
    for (unsigned num_ins = 0; num_ins <= max_ins; num_ins++) {
        btllib::BlindSeedNtHash hash_fn_copy(hash_fn);
        for (const auto base : editor) {
            const auto pos = hash_fn_copy.get_pos();
            if (pos >= signature.get_num_rows()) {
                break;
            }
            hash_fn_copy.roll(base);
            const auto col = i_seed * (max_ins + 1) + num_ins + 1;
            signature.set(pos, col, kmer_model->score(hash_fn_copy.hashes()));
        }
        hash_fn.roll('N');
    }
}

}

namespace aiedit {

ModelInterface::ModelInterface(const std::string_view seq,
                               size_t start,
                               size_t end,
                               unsigned max_edits,
                               const std::shared_ptr<KmerModel>& kmer_model)
  : prefix_kmer(seq.substr(start - 1, kmer_model->get_kmer_size()))
  , editor(seq, start + kmer_model->get_kmer_size() - 1, end + kmer_model->get_kmer_size() - 1)
  , max_edits(max_edits)
  , kmer_model(kmer_model)
{}

Buffer2D ModelInterface::get_signature()
{
    const auto num_features = kmer_model->get_seeds().size() * (max_edits + 1) + 1;
    Buffer2D signature(editor.get_size(), num_features);
    fill_repeats_row(signature, prefix_kmer, editor);
    for (unsigned i = 0; i < kmer_model->get_seeds().size(); i++) {
        fill_seed_row(signature, i, prefix_kmer, editor, kmer_model, max_edits);
    }
    return signature;
}

}
