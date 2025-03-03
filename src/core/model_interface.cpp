#include "model_interface.hpp"

#include <stdexcept>

namespace {

constexpr auto BASES = "ACGT";

}

namespace aiedit {

ModelInterface::ModelInterface(const std::string_view seq,
                               size_t start,
                               size_t end,
                               unsigned max_edits,
                               const std::shared_ptr<KmerModel>& kmer_model)
  : prefix_kmer(seq.substr(start - 1, kmer_model->get_kmer_size()))
  , editor(seq, start + kmer_model->get_kmer_size() - 1, end + kmer_model->get_kmer_size() - 1)
  , edits_left(max_edits)
  , kmer_model(kmer_model)
{}

std::optional<Edit> ModelInterface::operator()(unsigned output_index)
{
    if (is_terminated()) {
        throw std::runtime_error("[aiedit::ModelInterface] Interface has been terminated");
    }
    --edits_left;
    const auto pos = editor.get_position();
    if (output_index == 0) {
        editor.skip();
        return {};
    } else if (output_index <= 4) {
        const auto new_base = BASES[output_index - 1];
        editor.substitute(new_base);
        return Edit::substitution(pos, new_base);
    } else if (output_index <= 8) {
        const auto base = BASES[output_index - 5];
        editor.insert(base);
        return Edit::insertion(pos, base);
    } else if (output_index == 9) {
        editor.delete_base();
        return Edit::deletion(pos);
    } else if (output_index == 10) {
        terminate();
        return {};
    } else {
        const std::string idx_str = std::to_string(output_index);
        throw std::runtime_error("[aiedit::ModelInterface] Invalid model output: " + idx_str);
    }
}

void ModelInterface::terminate() { edits_left = 0; }

bool ModelInterface::is_terminated() const
{
    return edits_left == 0 || editor.get_num_remaining() == 0;
}

unsigned ModelInterface::get_num_edits_left() const { return edits_left; }

std::array<float, 4> ModelInterface::get_next_probs()
{
    btllib::BlindNtHash hash_fn(prefix_kmer,
                                kmer_model->get_num_hashes(),
                                kmer_model->get_kmer_size());
    for (const auto base : editor.get_consumed()) {
        hash_fn.roll(base);
    }
    std::array<float, 4> next_probs;
    constexpr char NEXT_BASE[] = "ACGT";
    for (unsigned i = 0; i < 4; i++) {
        if (editor.get_current() == NEXT_BASE[i]) {
            next_probs[i] = -1.0;
        } else {
            const char kmer_start = prefix_kmer[hash_fn.get_pos()];
            hash_fn.roll(NEXT_BASE[i]);
            next_probs[i] = kmer_model->score(hash_fn.hashes());
            hash_fn.roll_back(kmer_start);
        }
    }
    return next_probs;
}

Buffer2D ModelInterface::get_signature()
{
    Buffer2D signature(editor.get_size(), kmer_model->seeds.size() + 1);
    btllib::BlindSeedNtHash hash_fn(prefix_kmer.data(),
                                    kmer_model->seeds,
                                    kmer_model->get_num_hashes(),
                                    kmer_model->get_kmer_size());
    char prev = prefix_kmer[prefix_kmer.size() - 1];
    for (const auto base : editor) {
        const auto pos = hash_fn.get_pos();
        signature.set(pos, 0, base == prev ? 1.0f : 0.0f);
        prev = base;
        hash_fn.roll(base);
        for (size_t seed = 0; seed < kmer_model->seeds.size(); seed++) {
            const uint64_t* hashes = hash_fn.hashes() + (seed * kmer_model->get_num_hashes());
            signature.set(pos, seed + 1, kmer_model->score(hashes));
        }
    }
    return signature;
}

Buffer2D ModelInterface::encode_seeds(const std::vector<std::string>& seeds)
{
    Buffer2D x_seeds(seeds[0].size(), seeds.size());
    for (size_t i = 0; i < x_seeds.get_num_rows(); i++) {
        for (size_t j = 0; j < x_seeds.get_num_cols(); j++) {
            x_seeds.set(i, j, seeds[j][i] == '1' ? 1.0f : 0.0f);
        }
    }
    return x_seeds;
}

}
