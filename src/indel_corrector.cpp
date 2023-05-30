#include "indel_corrector.hpp"

namespace aiedit {

std::vector<Edit> IndelCorrector::fix(SequenceIterator& seq_iter, const Pattern& pattern)
{
    seq_iter.get_position();
    pattern.get_length();
    return std::vector<Edit>();
}

}  // namespace aiedit