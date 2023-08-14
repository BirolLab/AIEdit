#include "error_corrector.hpp"

#include <queue>

namespace {

using namespace aiedit;

const char ALPHABET[4] = {'A', 'C', 'G', 'T'};

inline unsigned count(SequenceIterator& seq_iter, const btllib::CountingBloomFilter8& bf)
{
    unsigned sum = 0;
    for (unsigned i = 0; i < seq_iter.get_num_seeds(); i++) {
        const auto c = bf.contains(seq_iter.get_hashes(i));
        if (c == 0) {
            return 0;
        }
        sum += c;
    }
    return sum;
}

inline bool permute(SequenceIterator& seq_iter, const btllib::CountingBloomFilter8& bf)
{
    char original = seq_iter.get_current();
    char fix = original;
    unsigned max_count = 0;
    for (const auto c : ALPHABET) {
        seq_iter.substitute_last(c);
        const auto count_c = count(seq_iter, bf);
        if (count_c > max_count) {
            fix = c;
            max_count = count_c;
        }
    }
    seq_iter.substitute_last(fix);
    return max_count > 0;
}

inline bool fix_mismatch(SequenceIterator& seq_iter,
                         const btllib::CountingBloomFilter8& bf,
                         std::vector<Edit>& edits)
{
    const auto before = seq_iter.get_current();
    const auto fixed = permute(seq_iter, bf);
    const auto after = seq_iter.get_current();
    if (fixed) {
        edits.emplace_back(seq_iter.get_position(), Edit::Type::MISMATCH, before, after);
    }
    return fixed;
}

inline bool fix_insertion(SequenceIterator& seq_iter,
                          const btllib::CountingBloomFilter8& bf,
                          std::vector<Edit>& edits)
{
    seq_iter.insert_last('A');
    const auto fixed = permute(seq_iter, bf);
    if (fixed) {
        const auto inserted = seq_iter.get_current();
        edits.emplace_back(seq_iter.get_position(), Edit::Type::INSERTION, '.', inserted);
    }
    return fixed;
}

inline void fix_deletion(SequenceIterator& seq_iter, std::vector<Edit>& edits, unsigned consecutive)
{
    const auto original = seq_iter.get_current();
    seq_iter.delete_last();
    edits.emplace_back(seq_iter.get_position() + consecutive, Edit::Type::DELETION, original, '.');
}

}  // namespace

namespace aiedit {

std::vector<Edit> ErrorCorrector::fix(SequenceIterator seq_iter, const Pattern& pattern)
{
    std::vector<Edit> edits;
    bool clean = true;
    const unsigned num_checks = pattern.get_length() * 2;
    unsigned consecutive_deletions = 0;
    for (unsigned i = 0; i < num_checks && clean; i++) {
        if (count(seq_iter, bf) > 0) {
            ;  // do nothing
        } else if (i >= pattern.get_length()) {
            clean = count(seq_iter, bf);
        } else if (pattern.get(i) == Edit::NONE) {
            clean = false;
        } else if (pattern.get(i) == Edit::MISMATCH) {
            seq_iter.next(consecutive_deletions);
            clean = fix_mismatch(seq_iter, bf, edits);
            consecutive_deletions = 0;
        } else if (pattern.get(i) == Edit::INSERTION) {
            seq_iter.next(consecutive_deletions);
            clean = fix_insertion(seq_iter, bf, edits);
            consecutive_deletions = 0;
        } else if (pattern.get(i) == Edit::DELETION) {
            fix_deletion(seq_iter, edits, consecutive_deletions);
            ++consecutive_deletions;
            continue;
        }
        if (!seq_iter.next()) {
            break;
        }
    }
    if (!clean) {
        edits.clear();
    }
    return edits;
}

}  // namespace aiedit