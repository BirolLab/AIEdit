#ifndef POLISHER_HPP
#define POLISHER_HPP

#include <btllib/counting_bloom_filter.hpp>
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
    unsigned get_num_ignored_patterns() const { return ignored_positions.size(); }
    unsigned get_num_mismatches() const { return num_mismatches; }
    unsigned get_num_insertions() const { return num_insertions; }
    unsigned get_num_deletions() const { return num_deletions; }
    const std::vector<Edit>& get_edits() const { return edits; }
    const std::vector<unsigned>& get_ignored_positions() const { return ignored_positions; }

  private:

    PolishingResults() = default;

    std::vector<Edit> edits;
    std::vector<unsigned> ignored_positions;
    unsigned num_fixed_patterns = 0;
    unsigned num_mismatches = 0;
    unsigned num_insertions = 0;
    unsigned num_deletions = 0;
};

class Polisher
{

  public:

    Polisher(const std::vector<std::string>& patterns,
             const btllib::CountingBloomFilter8& bf,
             const fdeep::model& model)
      : patterns(patterns)
      , bf(bf)
      , model(model)
    {}

    PolishingResults polish(SequenceIterator& seq_iter);

  private:

    const std::vector<std::string>& patterns;
    const btllib::CountingBloomFilter8& bf;
    const fdeep::model& model;

    static void update_results(const std::vector<Edit>& mismatches,
                               const std::vector<Edit>& indels,
                               unsigned seq_iter_position,
                               PolishingResults& results);
};

}  // namespace aiedit

#endif  // POLISHER_HPP