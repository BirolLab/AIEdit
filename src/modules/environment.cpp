#include "environment.hpp"

#include <stdexcept>

namespace aiedit {

Environment::Environment(const std::string_view seq,
                         size_t start,
                         size_t end,
                         unsigned max_edits,
                         std::shared_ptr<KmerModel> kmer_model)
  : prefix(seq.substr(start - 1, kmer_model->get_kmer_size()))
  , editor(seq, start + kmer_model->get_kmer_size() - 1, end + kmer_model->get_kmer_size() - 1)
  , max_edits(max_edits)
  , kmer_model(kmer_model)
{}

void Environment::act(Edit::Type edit_type, char new_base)
{
    if (is_terminated()) {
        throw std::runtime_error("Environment has been terminated");
    }
    if (edit_type == Edit::Type::SUBSTITUTE) {
        edits.emplace_back(Edit::substitution(editor.get_position(), new_base));
        editor.substitute(new_base);
    } else if (edit_type == Edit::Type::INSERT) {
        edits.emplace_back(Edit::insertion(editor.get_position(), new_base));
        editor.insert(new_base);
    } else if (edit_type == Edit::Type::DELETE) {
        edits.emplace_back(Edit::deletion(editor.get_position()));
        editor.delete_base();
    } else {
        editor.skip();
    }
}

void Environment::terminate() { max_edits = 0; }

bool Environment::is_terminated() const
{
    return edits.size() >= max_edits || editor.get_size() == 0;
}

std::array<float, 4> Environment::get_next_probs()
{
    btllib::BlindNtHash hash_fn(prefix, kmer_model->get_num_hashes(), kmer_model->get_kmer_size());
    for (const auto base : editor.get_consumed()) {
        hash_fn.roll(base);
    }
    std::array<float, 4> next_probs;
    constexpr char NEXT_BASE[] = "ACGT";
    for (unsigned i = 0; i < 4; i++) {
        if (editor.get_current() == NEXT_BASE[i]) {
            next_probs[i] = -1.0;
        } else {
            const char kmer_start = prefix[hash_fn.get_pos()];
            hash_fn.roll(NEXT_BASE[i]);
            next_probs[i] = kmer_model->score(hash_fn.hashes());
            hash_fn.roll_back(kmer_start);
        }
    }
    return next_probs;
}

Signature Environment::get_signature()
{
    Signature signature(editor.get_size(), kmer_model->seeds.size());
    btllib::BlindSeedNtHash hash_fn(prefix.data(),
                                    kmer_model->seeds,
                                    kmer_model->get_num_hashes(),
                                    kmer_model->get_kmer_size());
    for (const auto base : editor) {
        hash_fn.roll(base);
        for (size_t seed = 0; seed < kmer_model->seeds.size(); seed++) {
            const uint64_t* hashes = hash_fn.hashes() + (seed * kmer_model->get_num_hashes());
            signature.set(hash_fn.get_pos() - 1, seed, kmer_model->score(hashes));
        }
    }
    return signature;
}

}
