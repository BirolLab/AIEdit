#pragma once

#include "count_probabilities.hpp"
#include "edit.hpp"

namespace aiedit {

class EditFinder
{
  public:

    EditFinder(const CountProbabilities& cprobs);

    bool get_edits(const std::string& seq,
                   size_t pos,
                   unsigned k,
                   unsigned num_hashes,
                   const std::vector<Edit::Type>& pattern,
                   std::vector<Edit>& out_edits) const;

  private:

    const CountProbabilities& cprobs;
};

}