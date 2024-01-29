#ifndef POLISHER_HPP
#define POLISHER_HPP

#include <btllib/counting_bloom_filter.hpp>
#include <fdeep/fdeep.hpp>
#include <vector>

#include "edit.hpp"
#include "pattern.hpp"
#include "sequence_iterator.hpp"

namespace aiedit {

class PolishingResults
{
  public:

    void add_edits(const std::vector<Edit>& edits);

    const std::vector<Edit>& get_edits() const { return edits; }

    void add_ignored_pattern(unsigned position, const std::string& pattern);

    const std::vector<std::pair<unsigned, std::string>>& get_ignored_patterns() const
    {
        return ignored;
    }

    void merge(const PolishingResults& results);

    void sort_edits();
    void sort_ignored();

    unsigned get_num_fixed_patterns() const { return num_fixed_patterns; }
    unsigned get_num_ignored_patterns() const { return ignored.size(); }
    unsigned get_num_mismatches() const { return num_mismatches; }
    unsigned get_num_insertions() const { return num_insertions; }
    unsigned get_num_deletions() const { return num_deletions; }

    const std::string apply(const std::string& seq) const;

  private:

    std::vector<Edit> edits;
    std::vector<std::pair<unsigned, std::string>> ignored;
    unsigned num_fixed_patterns = 0;
    unsigned num_mismatches = 0;
    unsigned num_insertions = 0;
    unsigned num_deletions = 0;
};

class Polisher
{

  public:

    Polisher(const btllib::CountingBloomFilter8& bf, const fdeep::model& model)
      : bf(bf)
      , model(model)
    {}

    PolishingResults polish(SequenceIterator& seq_iter);

  private:

    const btllib::CountingBloomFilter8& bf;
    const fdeep::model& model;
};

}  // namespace aiedit

#endif  // POLISHER_HPP