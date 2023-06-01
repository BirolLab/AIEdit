#include "indel_corrector.hpp"
#include "mismatch_corrector.hpp"

namespace {

using namespace aiedit;

// inline std::vector<Edit> delete_bases(SequenceIterator& seq_iter, const Pattern& pattern) {}

}  // namespace

namespace aiedit {

std::vector<Edit> IndelCorrector::fix(SequenceIterator& seq_iter, const Pattern& pattern)
{
    seq_iter.get_base(0);
    pattern.get_length();
    return std::vector<Edit>();
}

}  // namespace aiedit