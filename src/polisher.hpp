#ifndef POLISHER_HPP
#define POLISHER_HPP

#include <btllib/bloom_filter.hpp>
#include <fdeep/fdeep.hpp>
#include <vector>

#include "edit.hpp"
#include "sequence_iterator.hpp"

namespace aiedit {

class PolishingResults
{
    friend class Polisher;

  public:

    unsigned get_num_fixed_patterns() const { return num_fixed_patterns; }
    unsigned get_num_ignored_patterns() const { return num_ignored_patterns; }
    unsigned get_num_mismatches() const { return num_mismatches; }
    unsigned get_num_insertions() const { return num_insertions; }
    unsigned get_num_deletions() const { return num_deletions; }
    const std::vector<Edit>& get_edits() const { return edits; }

  private:

    PolishingResults() = default;

    std::vector<Edit> edits;
    unsigned num_fixed_patterns = 0;
    unsigned num_ignored_patterns = 0;
    unsigned num_mismatches = 0;
    unsigned num_insertions = 0;
    unsigned num_deletions = 0;
};

class Polisher
{

  public:

    Polisher(unsigned pattern_length, const btllib::SeedBloomFilter& bf, const fdeep::model& model)
      : pattern_length(pattern_length)
      , bf(bf)
      , model(model)
    {}

    PolishingResults polish(SequenceIterator& seq_iter);

  private:

    const unsigned pattern_length;
    const btllib::SeedBloomFilter& bf;
    const fdeep::model& model;

    static void apply_edits(SequenceIterator& seq_iter,
                            const std::vector<Edit>& edits,
                            PolishingResults& results);
};

}  // namespace aiedit

#endif  // POLISHER_HPP