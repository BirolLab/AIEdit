#ifndef AIEDIT_MISMATCH_CORRECTOR_HPP
#define AIEDIT_MISMATCH_CORRECTOR_HPP

#include <btllib/bloom_filter.hpp>

#include "pattern.hpp"
#include "sequence_iterator.hpp"

namespace aiedit {

class MismatchCorrector
{
  public:

    MismatchCorrector(const btllib::SeedBloomFilter& bf) : bf(bf) {}

    /**
     * Find fixes for the mismatches in the current position of the sequence iterator
     * @param seq_iter Sequence iterator pointing to the mismatch region
     * @return `true` if any edits were applied
     */
    std::vector<Edit> get_fixes(SequenceIterator& seq_iter, const Pattern& pattern);

  private:

    const btllib::SeedBloomFilter& bf;
};

}  // namespace aiedit

#endif  // AIEDIT_MISMATCH_CORRECTOR_HPP