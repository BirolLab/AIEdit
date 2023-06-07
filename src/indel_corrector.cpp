#include "indel_corrector.hpp"
#include "mismatch_corrector.hpp"

namespace {

using namespace aiedit;

inline bool check_fixes(SequenceIterator& seq_iter,
                        const btllib::CountingBloomFilter8& bf,
                        unsigned pattern_length)
{
    auto bf_check = [&](const std::vector<uint64_t>& h) { return bf.contains(h) == 0; };
    SequenceIterator seq_iter_copy(seq_iter);
    unsigned signature_length = pattern_length + seq_iter.get_seed_length() - 1;
    while (signature_length-- > 0 && seq_iter_copy.has_next()) {
        seq_iter_copy.next();
        const auto& hashes = seq_iter_copy.get_hashes();
        if (std::any_of(hashes.begin(), hashes.end(), bf_check)) {
            return false;
        }
    }
    return true;
}

inline std::vector<Edit>
fix_deletions(SequenceIterator& seq_iter, Pattern& pattern, const btllib::CountingBloomFilter8& bf)
{
    std::vector<Edit> edits;
    unsigned num_deletions = 0;
    const unsigned n_del = pattern.get_length();  // pattern.get_count(Edit::Type::DELETION);
    bool fixed = false;
    std::string before = seq_iter.get_sequence().substr(seq_iter.get_position(), n_del);
    for (unsigned i = 1; i <= n_del && !fixed; i++) {
        seq_iter.remove(seq_iter.get_position());
        ++num_deletions;
        fixed = check_fixes(seq_iter, bf, pattern.get_length());
    }
    if (!fixed) {
        seq_iter.insert(seq_iter.get_position(), before.substr(0, num_deletions));
        return edits;
    }
    for (unsigned i = 0; i < num_deletions; i++) {
        edits.emplace_back(seq_iter.get_position() + i, Edit::Type::DELETION, before[i], '.');
    }
    return edits;
}

inline std::vector<Edit>
fix_insertions(SequenceIterator& seq_iter, Pattern& pattern, const btllib::CountingBloomFilter8& bf)
{
    std::vector<Edit> edits;
    unsigned num_insertions = 0;
    bool fixed = false;
    for (unsigned i = 1; i <= pattern.get_length() && !fixed; i++) {
        seq_iter.insert(seq_iter.get_position(), 'A');
        ++num_insertions;
        MismatchCorrector corrector(bf);
        Pattern mismatches(pattern.get_length());
        for (unsigned j = 0; j < i; j++) {
            mismatches.set(j, Edit::Type::MISMATCH);
        }
        corrector.fix(seq_iter, mismatches);
        fixed = check_fixes(seq_iter, bf, pattern.get_length());
    }
    if (!fixed) {
        seq_iter.remove(seq_iter.get_position(), num_insertions);
        return edits;
    }
    for (unsigned i = 0; i < num_insertions; i++) {
        const auto pos = seq_iter.get_position() + i;
        edits.emplace_back(pos, Edit::Type::INSERTION, '.', seq_iter.get_base(pos));
    }
    return edits;
}

}  // namespace

namespace aiedit {

std::vector<Edit> IndelCorrector::fix(SequenceIterator& seq_iter, Pattern& pattern)
{
    std::vector<Edit> edits;
    edits = fix_deletions(seq_iter, pattern, bf);
    if (edits.size() == 0) {
        edits = fix_insertions(seq_iter, pattern, bf);
    }
    return edits;
}

}  // namespace aiedit