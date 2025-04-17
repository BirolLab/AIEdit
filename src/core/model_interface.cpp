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

inline float get_score(const std::string& prefix_kmer,
                       aiedit::Editor& editor,
                       const std::shared_ptr<aiedit::KmerModel>& kmer_model)
{
    float score = 0;
    btllib::BlindNtHash hash_fn(prefix_kmer,
                                kmer_model->get_num_hashes(),
                                kmer_model->get_kmer_size());
    for (const auto c : editor) {
        hash_fn.roll(c);
        score += kmer_model->score(hash_fn.hashes());
    }
    return score / editor.get_size();
}

inline std::optional<std::string>
find_insertions(const std::string& prefix_kmer,
                aiedit::Editor& editor,
                const std::shared_ptr<aiedit::KmerModel>& kmer_model,
                unsigned num_ins)
{
    const auto score = get_score(prefix_kmer, editor, kmer_model);
    btllib::BlindNtHash hash_fn(prefix_kmer,
                                kmer_model->get_num_hashes(),
                                kmer_model->get_kmer_size());
    std::string result;
    result.reserve(2 * num_ins);
    while (num_ins--) {
        for (unsigned i = 0; i < 4; i++) {
            hash_fn.peek(BASES[i]);
            if (kmer_model->score(hash_fn.hashes()) > 0.5) {
                editor.insert(BASES[i]);
                hash_fn.roll(BASES[i]);
                result.push_back('+');
                result.push_back(BASES[i]);
                break;
            }
        }
    }
    if (get_score(prefix_kmer, editor, kmer_model) <= score) {
        return {};
    } else {
        return result;
    }
}

inline std::optional<std::string>
find_deletions(const std::string& prefix_kmer,
               aiedit::Editor& editor,
               const std::shared_ptr<aiedit::KmerModel>& kmer_model,
               unsigned num_del)
{
    const auto score = get_score(prefix_kmer, editor, kmer_model);
    std::string result;
    result.reserve(num_del);
    while (num_del-- && editor.get_num_remaining() > 0) {
        editor.delete_base();
        result.push_back('-');
    }
    if (get_score(prefix_kmer, editor, kmer_model) <= score) {
        return {};
    } else {
        return result;
    }
}

inline std::optional<std::string>
find_mismatches(const std::string& prefix_kmer,
                aiedit::Editor& editor,
                const std::shared_ptr<aiedit::KmerModel>& kmer_model,
                float* mismatches,
                unsigned num_mis)
{
    const auto score = get_score(prefix_kmer, editor, kmer_model);
    btllib::BlindNtHash hash_fn(prefix_kmer,
                                kmer_model->get_num_hashes(),
                                kmer_model->get_kmer_size());
    std::string result;
    result.reserve(num_mis);
    // TODO optimize
    bool has_snp = false;
    for (unsigned pos = 0; pos < num_mis && editor.get_num_remaining() > 0; pos++) {
        if (mismatches[pos] < 0) {
            hash_fn.roll(editor.get_current());
            result.push_back('*');
            editor.skip();
            continue;
        }
        has_snp = true;
        bool found = false;
        for (unsigned i = 0; i < 4; i++) {
            if (BASES[i] != editor.get_current()) {
                hash_fn.peek(BASES[i]);
                if (kmer_model->score(hash_fn.hashes()) > 0.5) {
                    editor.substitute(BASES[i]);
                    hash_fn.roll(BASES[i]);
                    result.push_back(BASES[i]);
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            return {};
        }
    }
    if (!has_snp || get_score(prefix_kmer, editor, kmer_model) <= score) {
        return {};
    } else {
        return result;
    }
}

inline size_t argmax(const float* array, size_t size)
{
    size_t i_max = 0;
    float max_value = array[0];
    for (size_t i = 1; i < size; i++) {
        if (array[i] > max_value) {
            max_value = array[i];
            i_max = i;
        }
    }
    return i_max;
}

}

namespace aiedit {

ModelInterface::ModelInterface(const std::string_view seq,
                               size_t start,
                               size_t end,
                               unsigned max_indels,
                               const std::shared_ptr<KmerModel>& kmer_model)
  : prefix_kmer(seq.substr(start - 1, kmer_model->get_kmer_size()))
  , editor(seq, start + kmer_model->get_kmer_size() - 1, end + kmer_model->get_kmer_size() - 1)
  , max_indels(max_indels)
  , kmer_model(kmer_model)
{}

Buffer2D ModelInterface::get_signature()
{
    const auto num_features = kmer_model->get_seeds().size() * (max_indels + 1) + 1;
    Buffer2D signature(editor.get_size(), num_features);
    fill_repeats_row(signature, prefix_kmer, editor);
    for (unsigned i = 0; i < kmer_model->get_seeds().size(); i++) {
        fill_seed_row(signature, i, prefix_kmer, editor, kmer_model, max_indels);
    }
    return signature;
}

std::optional<std::string> ModelInterface::update(const std::vector<float*>& outputs,
                                                  const std::vector<long>& sizes)
{
    const auto indel_prob = outputs[0][0];
    const auto indels = argmax(outputs[2], sizes[2]);
    const auto mismatches = outputs[1];
    const auto max_mismatches = sizes[1];
    if (indel_prob > 0 && indels < max_indels) {
        return find_insertions(prefix_kmer, editor, kmer_model, indels + 1);
    } else if (indel_prob > 0) {
        return find_deletions(prefix_kmer, editor, kmer_model, indels - max_indels + 1);
    } else {
        return find_mismatches(prefix_kmer, editor, kmer_model, mismatches, max_mismatches);
    }
}

}
