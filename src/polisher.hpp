#ifndef POLISHER_HPP
#define POLISHER_HPP

#include <btllib/counting_bloom_filter.hpp>
#include <fdeep/fdeep.hpp>
#include <vector>

#include "edit.hpp"
#include "sequence_iterator.hpp"

namespace aiedit {

using IgnoredPatternsList = std::vector<std::pair<unsigned, std::string>>;

class PolishingResults
{
    friend class Polisher;

  public:

    unsigned get_num_fixed_patterns() const { return num_fixed_patterns; }
    unsigned get_num_ignored_patterns() const { return ignored_patterns.size(); }
    unsigned get_num_mismatches() const { return num_mismatches; }
    unsigned get_num_insertions() const { return num_insertions; }
    unsigned get_num_deletions() const { return num_deletions; }
    const std::vector<Edit>& get_edits() const { return edits; }
    const IgnoredPatternsList& get_ignored_patterns() const { return ignored_patterns; }

  private:

    PolishingResults() = default;

    std::vector<Edit> edits;
    IgnoredPatternsList ignored_patterns;
    unsigned num_fixed_patterns = 0;
    unsigned num_mismatches = 0;
    unsigned num_insertions = 0;
    unsigned num_deletions = 0;
};

class Polisher
{

  public:

    Polisher(unsigned pattern_length,
             const btllib::CountingBloomFilter8& bf,
             const fdeep::model& model)
      : pattern_length(pattern_length)
      , bf(bf)
      , model(model)
    {}

    PolishingResults polish(SequenceIterator& seq_iter);

  private:

    const unsigned pattern_length;
    const btllib::CountingBloomFilter8& bf;
    const fdeep::model& model;

    static void update_results(const std::vector<Edit>& mismatches,
                               const std::vector<Edit>& indels,
                               unsigned seq_iter_position,
                               const std::string& pattern_string,
                               PolishingResults& results);
};

}  // namespace aiedit

#endif  // POLISHER_HPP