#include "gap_filler.hpp"

#include <deque>

namespace {

constexpr auto BASES = "ACGT";

inline float get_score(const std::string_view seq,
                       size_t start_kmer,
                       const std::string& insertion,
                       const std::string_view suffix,
                       const std::shared_ptr<aiedit::KmerModel>& kmer_model)
{
    float score = 0;
    const std::string prefix_kmer(seq.data() + start_kmer - 1,
                                  seq.data() + start_kmer - 1 + kmer_model->get_kmer_size());
    btllib::BlindNtHash hash_fn(prefix_kmer,
                                kmer_model->get_num_hashes(),
                                kmer_model->get_kmer_size());
    for (const auto c : insertion) {
        hash_fn.roll(c);
        score += kmer_model->score(hash_fn.hashes());
    }
    for (const auto c : suffix) {
        hash_fn.roll(c);
        score += kmer_model->score(hash_fn.hashes());
    }
    return score / (float)(insertion.size() + suffix.size());
}

}

namespace aiedit {

GapFiller::GapFiller(const std::shared_ptr<KmerModel>& kmer_model,
                     unsigned max_size,
                     float min_score,
                     size_t suffix_size)
  : kmer_model(kmer_model)
  , max_size(max_size)
  , min_score(min_score)
  , suffix_size(suffix_size)
{}

unsigned GapFiller::get_max_size() const { return max_size; }

std::pair<float, std::string>
GapFiller::fill(const std::string_view seq, size_t start, size_t end) const
{
    const auto kmer_size = kmer_model->get_kmer_size();
    if (end * 2 + kmer_size - start >= seq.size()) {
        return {0.0, ""};
    }
    const std::string prefix_kmer(seq.data() + start - 1, seq.data() + start - 1 + kmer_size);
    const std::string_view suffix(seq.data() + end, suffix_size);
    btllib::BlindNtHash hash_fn(prefix_kmer, kmer_model->get_num_hashes(), kmer_size);
    std::string insertion;
    float insertion_score = 0;
    while (insertion.size() < end - start - kmer_size + max_size) {
        bool found = false;
        float score = insertion_score;
        btllib::BlindNtHash hash_fn_copy(hash_fn);
        for (const auto c : suffix) {
            hash_fn_copy.roll(c);
            score += kmer_model->score(hash_fn.hashes());
        }
        score /= (float)(insertion.size() + suffix.size());
        if (score > min_score) {
            return {score, insertion};
        }
        for (unsigned i = 0; i < 4 && !found; i++) {
            hash_fn.peek(BASES[i]);
            const auto kmer_score = kmer_model->score(hash_fn.hashes());
            if (kmer_score > min_score) {
                hash_fn.roll(BASES[i]);
                insertion.push_back(BASES[i]);
                insertion_score += kmer_score;
                found = true;
            }
        }
        if (!found) {
            return {0.0, ""};
        }
    }
    return {0.0, ""};
}

}