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

inline std::vector<Edit> fix_mismatches(SequenceIterator& seq_iter,
                                        const Pattern& pattern,
                                        const btllib::CountingBloomFilter8& bf)
{
    std::vector<Edit> edits;
    for (unsigned i = 0; i < pattern.get_length(); i++) {
        if (pattern.get(i) == Edit::Type::MISMATCH) {
            const auto before = std::string(1, seq_iter.get_current());
            const auto fixed = permute(seq_iter, bf);
            if (fixed) {
                const std::string after(1, seq_iter.get_current());
                edits.emplace_back(seq_iter.get_position(), Edit::Type::MISMATCH, before, after);
            } else {
                return std::vector<Edit>();
            }
        } else if (count(seq_iter, bf) == 0) {
            return std::vector<Edit>();
        }
        if (!seq_iter.next()) {
            break;
        }
    }
    return edits;
}

inline std::vector<Edit> fix_insertions(SequenceIterator& seq_iter,
                                        const Pattern& pattern,
                                        const btllib::CountingBloomFilter8& bf)
{
    const auto position = seq_iter.get_position();
    const std::string before(1, seq_iter.get_current());
    std::string inserted;
    for (unsigned i = 0; i < pattern.get_length(); i++) {
        if (pattern.get(i) == Edit::Type::INSERTION) {
            seq_iter.insert_last('A');
            const auto fixed = permute(seq_iter, bf);
            if (fixed) {
                inserted.push_back(seq_iter.get_current());
            } else {
                return std::vector<Edit>();
            }
        } else if (count(seq_iter, bf) == 0) {
            return std::vector<Edit>();
        }
        if (!seq_iter.next()) {
            break;
        }
    }
    Edit edit(position, Edit::Type::INSERTION, before, inserted + before);
    return std::vector<Edit>(1, edit);
}

inline std::vector<Edit> fix_deletions(SequenceIterator& seq_iter,
                                       const Pattern& pattern,
                                       const btllib::CountingBloomFilter8& bf)
{
    const auto position = seq_iter.get_position();
    std::string before;
    for (unsigned i = 0; i < pattern.get_length(); i++) {
        if (pattern.get(i) == Edit::Type::DELETION) {
            before.push_back(seq_iter.get_current());
            if (seq_iter.has_next()) {
                seq_iter.delete_last();
            }
        } else if (count(seq_iter, bf) == 0) {
            return std::vector<Edit>();
        }
        if (!seq_iter.next()) {
            break;
        }
    }
    Edit edit(position, Edit::Type::DELETION, before, ".");
    return std::vector<Edit>(1, edit);
}

}  // namespace

namespace aiedit {

std::vector<Edit> ErrorCorrector::fix(SequenceIterator seq_iter, Pattern& pattern)
{
    if (pattern.get_count(Edit::Type::MISMATCH) > 0) {
        return fix_mismatches(seq_iter, pattern, bf);
    } else if (pattern.get_count(Edit::Type::INSERTION) > 0) {
        return fix_insertions(seq_iter, pattern, bf);
    } else if (pattern.get_count(Edit::Type::DELETION) > 0) {
        return fix_deletions(seq_iter, pattern, bf);
    }
    return std::vector<Edit>();
}

}  // namespace aiedit