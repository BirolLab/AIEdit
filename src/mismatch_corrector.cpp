#include "mismatch_corrector.hpp"

#include <queue>

namespace {

using namespace aiedit;

const char ALPHABET[4] = {'A', 'C', 'G', 'T'};

inline std::queue<unsigned> get_mismatch_positions(size_t base_position, const Pattern& pattern)
{
    std::queue<unsigned> positions;
    for (unsigned i = 0; i < pattern.get_length(); i++) {
        if (pattern.get(i) == Edit::Type::MISMATCH) {
            positions.push(base_position + i);
        }
    }
    return positions;
}

inline bool
check_fix(SequenceIterator seq_iter, const btllib::CountingBloomFilter8& bf, unsigned num_checks)
{
    auto bf_check = [&](const std::vector<uint64_t>& h) { return bf.contains(h) == 0; };
    seq_iter.previous();
    while (num_checks-- > 0 && seq_iter.has_next()) {
        seq_iter.next();
        const auto& hashes = seq_iter.get_hashes();
        if (std::any_of(hashes.begin(), hashes.end(), bf_check)) {
            return false;
        }
    }
    return true;
}

inline char fix_next(SequenceIterator& seq_iter,
                     unsigned position,
                     unsigned num_checks,
                     const btllib::CountingBloomFilter8& bf)
{
    const char original = seq_iter.get_base(position);
    for (const char c : ALPHABET) {
        if (c != original) {
            seq_iter.update(position, c);
            if (check_fix(seq_iter, bf, num_checks)) {
                return c;
            }
        }
    }
    return 'N';
}

inline void revert_edits(SequenceIterator& seq_iter, const std::vector<Edit>& edits)
{
    for (const auto& edit : edits) {
        seq_iter.update(edit.position, edit.before);
    }
}

}  // namespace

namespace aiedit {

std::vector<Edit> MismatchCorrector::fix(SequenceIterator& seq_iter, const Pattern& pattern)
{
    std::vector<Edit> edits;
    auto positions = get_mismatch_positions(seq_iter.get_position(), pattern);
    unsigned last_position = seq_iter.get_position() - 1;
    while (!positions.empty()) {
        char before = seq_iter.get_base(positions.front());
        unsigned num_checks = positions.front() - last_position;
        auto fix = fix_next(seq_iter, positions.front(), num_checks, bf);
        edits.emplace_back(positions.front(), Edit::Type::MISMATCH, before, fix);
        last_position = positions.front();
        positions.pop();
    }
    if (!check_fix(seq_iter, bf, seq_iter.get_seed_length() - 1)) {
        revert_edits(seq_iter, edits);
        edits.clear();
    }
    return edits;
}

}  // namespace aiedit