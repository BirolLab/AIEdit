#include "indel_corrector.hpp"
#include "mismatch_corrector.hpp"

namespace {

using namespace aiedit;

inline std::vector<Edit> fix_deletions(SequenceIterator& seq_iter, Pattern& pattern)
{
    std::vector<Edit> edits;
    unsigned num_deletions = pattern.get_count(Edit::Type::DELETION);
    std::string before = seq_iter.get_sequence().substr(seq_iter.get_position(), num_deletions);
    seq_iter.remove(seq_iter.get_position(), num_deletions);
    for (unsigned i = 0; i < before.size(); i++) {
        edits.emplace_back(seq_iter.get_position() + i, Edit::Type::DELETION, before[i], '.');
    }
    return edits;
}

inline std::vector<Edit>
fix_insertions(SequenceIterator& seq_iter, Pattern& pattern, const btllib::CountingBloomFilter8& bf)
{
    std::vector<Edit> edits;
    std::string gap(pattern.get_count(Edit::Type::INSERTION), 'A');
    seq_iter.insert(seq_iter.get_position(), gap);
    MismatchCorrector corrector(bf);
    Pattern mismatches(pattern.get_length());
    for (unsigned i = 0; i < gap.size(); i++) {
        mismatches.set(i, Edit::Type::MISMATCH);
    }
    corrector.fix(seq_iter, mismatches);
    for (unsigned i = 0; i < gap.size(); i++) {
        const auto pos = seq_iter.get_position() + i;
        edits.emplace_back(pos, Edit::Type::INSERTION, '.', seq_iter.get_base(pos));
    }
    return edits;
}

}  // namespace

namespace aiedit {

std::vector<Edit> IndelCorrector::fix(SequenceIterator& seq_iter, Pattern& pattern)
{
    if (pattern.get_count(Edit::Type::DELETION) > 0) {
        return fix_deletions(seq_iter, pattern);
    }
    return fix_insertions(seq_iter, pattern, bf);
}

}  // namespace aiedit