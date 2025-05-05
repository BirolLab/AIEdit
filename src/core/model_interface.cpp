#include "model_interface.hpp"

#include <math.h>
#include <queue>
#include <stdexcept>

namespace {

constexpr auto BASES = "ACGT";

inline void fill_repeats_row(aiedit::Buffer2D& signature,
                             const std::string_view seq,
                             size_t start_kmer,
                             unsigned kmer_size,
                             aiedit::Editor& editor)
{
    char prev = seq[start_kmer + kmer_size - 2];
    size_t pos = 0;
    for (const auto base : editor) {
        signature.set(pos++, 0, base == prev ? 1.0f : 0.0f);
        prev = base;
    }
}

inline void fill_seed_row(aiedit::Buffer2D& signature,
                          unsigned i_seed,
                          const std::string_view seq,
                          size_t start_kmer,
                          aiedit::Editor& editor,
                          const std::shared_ptr<aiedit::KmerModel>& kmer_model,
                          unsigned max_ins)
{
    btllib::BlindSeedNtHash hash_fn(seq.data(),
                                    {kmer_model->get_seeds()[i_seed]},
                                    kmer_model->get_num_hashes(),
                                    kmer_model->get_kmer_size(),
                                    start_kmer - 1);
    for (unsigned num_ins = 0; num_ins <= max_ins; num_ins++) {
        btllib::BlindSeedNtHash hash_fn_copy(hash_fn);
        for (const auto base : editor) {
            const auto pos = hash_fn_copy.get_pos() - start_kmer + 1;
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

inline unsigned get_num_mismatches(const float* probs, size_t max_mismatches)
{
    unsigned num_mis = max_mismatches;
    while (num_mis >= 0) {
        if (num_mis == 0 || probs[num_mis - 1] >= 0) {
            return num_mis;
        } else {
            --num_mis;
        }
    }
    return 0;
}

inline std::string find_mismatches(const std::string_view seq,
                                   size_t start_kmer,
                                   aiedit::Editor& editor,
                                   const std::shared_ptr<aiedit::KmerModel>& kmer_model,
                                   float* mismatches,
                                   unsigned num_mis)
{
    const std::string prefix_kmer(seq.data() + start_kmer - 1,
                                  seq.data() + start_kmer - 1 + kmer_model->get_kmer_size());
    btllib::BlindNtHash hash_fn(prefix_kmer,
                                kmer_model->get_num_hashes(),
                                kmer_model->get_kmer_size());
    std::string result;
    result.reserve(num_mis);
    // TODO optimize
    for (unsigned pos = 0; pos < num_mis && editor.get_num_remaining() > 0; pos++) {
        if (mismatches[pos] < 0) {
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
    return result;
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
  : seq(seq)
  , start_kmer(start)
  , editor(seq, start + kmer_model->get_kmer_size() - 1, end + kmer_model->get_kmer_size() - 1)
  , max_indels(max_indels)
  , kmer_model(kmer_model)
{}

Buffer2D ModelInterface::get_signature()
{
    const auto num_features = kmer_model->get_seeds().size() * (max_indels + 1) + 1;
    Buffer2D signature(editor.get_size(), num_features);
    fill_repeats_row(signature, seq, start_kmer, kmer_model->get_kmer_size(), editor);
    for (unsigned i = 0; i < kmer_model->get_seeds().size(); i++) {
        fill_seed_row(signature, i, seq, start_kmer, editor, kmer_model, max_indels);
    }
    return signature;
}

std::tuple<Edit::Type, std::string, float>
ModelInterface::update(const std::vector<float*>& outputs, const std::vector<long>& sizes)
{
    const auto indel_prob = outputs[0][0];
    const auto num_indels = argmax(outputs[2], sizes[2]);
    const auto mismatches = outputs[1];
    const auto num_mismatches = get_num_mismatches(mismatches, sizes[1]);
    Edit::Type edit_type;
    std::string edit;
    float score;
    if (indel_prob > 0 && num_indels < max_indels) {
        const auto result = find_deletions(seq, start_kmer, editor, kmer_model, max_indels);
        edit_type = Edit::Type::DELETE;
        edit = result.first;
        score = result.second;
    } else if (indel_prob > 0) {
        edit_type = Edit::Type::INSERT;
        edit = find_insertions(seq, start_kmer, editor, kmer_model, num_indels - max_indels + 1);
        score = get_score(seq, start_kmer, editor, kmer_model);
    } else {
        edit_type = Edit::Type::SUBSTITUTE;
        edit = find_mismatches(seq, start_kmer, editor, kmer_model, mismatches, num_mismatches);
        score = get_score(seq, start_kmer, editor, kmer_model);
    }
    return std::tuple(edit_type, edit, score);
}

}
