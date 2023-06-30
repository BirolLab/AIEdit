#include "mismatch_corrector.hpp"

#include <queue>

namespace {

using namespace aiedit;

const char ALPHABET[4] = {'A', 'C', 'G', 'T'};

inline unsigned count(SequenceIterator& seq_iter, const btllib::CountingBloomFilter8& bf)
{
    unsigned sum = 0;
    for (unsigned i = 0; i < seq_iter.get_num_seeds(); i++) {
        sum += bf.contains(seq_iter.get_hashes(i));
    }
    return sum;
}

inline bool permute(SequenceIterator& seq_iter, const btllib::CountingBloomFilter8& bf)
{
    char original = seq_iter.get_current();
    char fix = 'N';
    unsigned max_count = 0;
    for (const auto c : ALPHABET) {
        if (c == original) {
            continue;
        }
        seq_iter.substitute_last(c);
        const auto count_c = count(seq_iter, bf);
        if (count_c > 0 && count_c > max_count) {
            fix = c;
            max_count = count_c;
        }
    }
    seq_iter.substitute_last(fix);
    return fix;
}

}  // namespace

namespace aiedit {

std::vector<Edit> MismatchCorrector::fix(SequenceIterator& seq_iter, const Pattern& pattern)
{
    std::vector<Edit> edits;
    const unsigned num_checks = seq_iter.get_seed_length() + pattern.get_length() - 1;
    for (unsigned i = 0; i < num_checks; i++) {
        if (i < pattern.get_length() && pattern.get(i) == Edit::MISMATCH) {
            const auto before = seq_iter.get_current();
            const auto fixed = permute(seq_iter, bf);
            const auto after = seq_iter.get_current();
            if (fixed) {
                edits.emplace_back(seq_iter.get_position(), Edit::Type::MISMATCH, before, after);
            } else {
                seq_iter.skip_kmer();
                return edits;
            }
        } else if (count(seq_iter, bf) == 0) {
            seq_iter.skip_kmer();
            return edits;
        }
    }
    return edits;
}

}  // namespace aiedit