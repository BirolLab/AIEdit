#include "gap_filler.hpp"

namespace {

constexpr auto BASES = "ACGT";

}

namespace aiedit {

GapFiller::GapFiller(const std::shared_ptr<KmerModel>& kmer_model,
                     unsigned max_size,
                     float min_score)
  : kmer_model(kmer_model)
  , max_size(max_size)
  , min_score(min_score)
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
    const std::string_view suffix_kmer(seq.data() + end + kmer_size, end - start);
    const auto suffix_size = suffix_kmer.size();
    btllib::BlindNtHash hash_fn(prefix_kmer, kmer_model->get_num_hashes(), kmer_size);
    std::string insertion;
    float sum_score = 0;
    while (insertion.size() <= max_size + suffix_size) {
        bool found = false;
        for (unsigned i = 0; i < 4 && !found; i++) {
            hash_fn.peek(BASES[i]);
            const auto kmer_score = kmer_model->score(hash_fn.hashes());
            if (kmer_score > min_score) {
                hash_fn.roll(BASES[i]);
                insertion.push_back(BASES[i]);
                sum_score += kmer_score;
                found = true;
            }
        }
        if (!found) {
            return {0.0, ""};
        }
        if (insertion.size() >= suffix_size) {
            std::string_view sv(insertion.data() + insertion.size() - suffix_size, suffix_size);
            if (sv == suffix_kmer) {
                break;
            }
        }
    }
    return {sum_score / (float)insertion.size(),
            insertion.substr(0, insertion.size() - suffix_size)};
}

}