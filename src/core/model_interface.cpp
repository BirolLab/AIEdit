#include "model_interface.hpp"

#include <math.h>
#include <queue>
#include <stdexcept>

namespace {

constexpr auto BASES = "ACGT";

inline void fill_repeats_row(aiedit::Buffer2D& signature,
                             const std::string_view seq,
                             size_t start_kmer,
                             unsigned kmer_size)
{
    char prev = seq[start_kmer + kmer_size - 2];
    for (unsigned pos = 0; pos < signature.get_num_rows(); pos++) {
        const auto base = seq[pos + start_kmer + kmer_size - 1];
        signature.set(pos, 0, base == prev ? 1.0f : 0.0f);
        prev = base;
    }
}

inline void fill_seed_row(aiedit::Buffer2D& signature,
                          unsigned i_seed,
                          const std::string_view seq,
                          size_t start_kmer,
                          const std::shared_ptr<aiedit::KmerModel>& kmer_model,
                          unsigned max_ins)
{
    const auto k = kmer_model->get_kmer_size();
    btllib::BlindSeedNtHash hash_fn(seq.data(),
                                    {kmer_model->get_seeds()[i_seed]},
                                    kmer_model->get_num_hashes(),
                                    k,
                                    start_kmer - 1);
    for (unsigned num_ins = 0; num_ins <= max_ins; num_ins++) {
        btllib::BlindSeedNtHash hash_fn_copy(hash_fn);
        for (unsigned pos = 0; pos < signature.get_num_rows() && pos + k < seq.length(); pos++) {
            hash_fn_copy.roll(seq[pos + start_kmer + k - 1]);
            const auto col = i_seed * (max_ins + 1) + num_ins + 1;
            signature.set(pos, col, kmer_model->query_seed(hash_fn_copy.hashes()));
        }
        hash_fn.roll('N');
    }
}

inline float get_score(const std::string_view seq,
                       size_t start_kmer,
                       aiedit::Editor& editor,
                       const std::shared_ptr<aiedit::KmerModel>& kmer_model)
{
    float score = 0;
    const std::string prefix_kmer(seq.data() + start_kmer - 1,
                                  seq.data() + start_kmer - 1 + kmer_model->get_kmer_size());
    btllib::BlindNtHash hash_fn(prefix_kmer,
                                kmer_model->get_num_hashes(),
                                kmer_model->get_kmer_size());
    for (const auto c : editor) {
        hash_fn.roll(c);
        score += std::log(kmer_model->score(hash_fn.hashes()));
    }
    return std::exp(score / editor.get_size());
}

inline std::string find_insertions(const std::string_view seq,
                                   size_t start_kmer,
                                   aiedit::Editor& editor,
                                   const std::shared_ptr<aiedit::KmerModel>& kmer_model,
                                   unsigned num_ins)
{
    const std::string prefix_kmer(seq.data() + start_kmer - 1,
                                  seq.data() + start_kmer - 1 + kmer_model->get_kmer_size());
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
                result.push_back(BASES[i]);
                break;
            }
        }
    }
    return result;
}

inline std::pair<std::string, float>
find_deletions(const std::string_view seq,
               size_t start_kmer,
               aiedit::Editor& editor,
               const std::shared_ptr<aiedit::KmerModel>& kmer_model,
               unsigned max_del)
{
    aiedit::Editor editor_copy(editor);
    std::priority_queue<std::pair<float, unsigned>> scores;
    for (unsigned num_del = 1; num_del <= max_del && editor_copy.get_num_remaining() > 0;
         num_del++) {
        editor_copy.delete_base();
        const auto score = get_score(seq, start_kmer, editor_copy, kmer_model);
        scores.push(std::make_pair(score, num_del));
    }
    return std::make_pair(std::string(scores.top().second, '-'), scores.top().first);
}

inline std::vector<bool> get_mismatch_pattern(unsigned i_edit)
{
    std::vector<bool> pattern = {true};
    while (i_edit > 0) {
        pattern.push_back(i_edit % 2 == 1);
        i_edit /= 2;
    }
    return pattern;
}

inline std::string find_mismatches(const std::string_view seq,
                                   size_t start_kmer,
                                   aiedit::Editor& editor,
                                   const std::shared_ptr<aiedit::KmerModel>& kmer_model,
                                   std::vector<bool> mismatches)
{
    const std::string prefix_kmer(seq.data() + start_kmer - 1,
                                  seq.data() + start_kmer - 1 + kmer_model->get_kmer_size());
    btllib::BlindNtHash hash_fn(prefix_kmer,
                                kmer_model->get_num_hashes(),
                                kmer_model->get_kmer_size());
    std::string result;
    result.reserve(mismatches.size());
    unsigned last_edited = 0;
    for (unsigned pos = 0; pos < mismatches.size() && editor.get_num_remaining() > 0; pos++) {
        if (!mismatches[pos]) {
            hash_fn.roll(editor.get_current());
            result.push_back(editor.get_current());
            editor.skip();
            continue;
        }
        bool found = false;
        char original = editor.get_current();
        for (unsigned i = 0; i < 4 && !found; i++) {
            hash_fn.peek(BASES[i]);
            if (kmer_model->score(hash_fn.hashes()) > 0.5) {
                if (BASES[i] != original) {
                    last_edited = pos + 1;
                }
                editor.substitute(BASES[i]);
                hash_fn.roll(BASES[i]);
                result.push_back(BASES[i]);
                found = true;
            }
        }
        if (!found) {
            break;
        }
        editor.skip();
    }
    return result.substr(0, last_edited);
}

}

namespace aiedit {

ModelInterface::ModelInterface(const std::string_view seq,
                               size_t start_kmer,
                               size_t end_kmer,
                               unsigned max_mismatches,
                               unsigned max_indels,
                               const std::shared_ptr<KmerModel>& kmer_model)
  : seq(seq)
  , start_kmer(start_kmer)
  , end_kmer(end_kmer)
  , max_mismatches(max_mismatches)
  , max_indels(max_indels)
  , kmer_model(kmer_model)
{}

Buffer2D ModelInterface::get_signature()
{
    const auto num_features = kmer_model->get_seeds().size() * (max_indels + 1) + 1;
    Buffer2D signature(end_kmer - start_kmer, num_features);
    fill_repeats_row(signature, seq, start_kmer, kmer_model->get_kmer_size());
    for (unsigned i = 0; i < kmer_model->get_seeds().size(); i++) {
        fill_seed_row(signature, i, seq, start_kmer, kmer_model, max_indels);
    }
    return signature;
}

std::tuple<Edit::Type, std::string, float> ModelInterface::update(unsigned i_edit)
{
    Edit::Type edit_type;
    std::string edit;
    float kmer_score;
    Editor editor(seq,
                  start_kmer + kmer_model->get_kmer_size() - 1,
                  end_kmer + kmer_model->get_kmer_size() - 1);
    const auto mismatch_indices = 1 << (max_mismatches - 1);
    if (i_edit < mismatch_indices) {
        edit_type = Edit::Type::SUBSTITUTE;
        const auto mismatches = get_mismatch_pattern(i_edit);
        edit = find_mismatches(seq, start_kmer, editor, kmer_model, mismatches);
        kmer_score = get_score(seq, start_kmer, editor, kmer_model);
    } else if (i_edit < mismatch_indices + max_indels) {
        edit_type = Edit::Type::DELETE;
        const auto num_del = i_edit - mismatch_indices + 1;
        const auto result = find_deletions(seq, start_kmer, editor, kmer_model, num_del);
        edit = result.first;
        kmer_score = result.second;
    } else {
        edit_type = Edit::Type::INSERT;
        const auto num_ins = i_edit - mismatch_indices - max_indels + 1;
        edit = find_insertions(seq, start_kmer, editor, kmer_model, num_ins);
        kmer_score = get_score(seq, start_kmer, editor, kmer_model);
    }
    return {edit_type, edit, kmer_score};
}

}
