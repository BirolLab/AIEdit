#pragma once

#include "count_probabilities.hpp"
#include "edit.hpp"

namespace aiedit {

class EditFinder
{
  public:

    EditFinder(const CountProbabilities& cprobs, unsigned kmer_size);

    bool get_edits(const std::string& seq,
                   size_t pos,
                   const std::vector<Edit::Type>& pattern,
                   std::vector<Edit>& out_edits) const;

  private:

    const CountProbabilities& cprobs;
    unsigned kmer_size;
};

}