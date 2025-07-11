#pragma once

#include <memory>
#include <string>

#include "kmer_model.hpp"

namespace aiedit {

class GapFiller
{

  public:

    GapFiller(const std::shared_ptr<KmerModel>& kmer_model, unsigned max_size, float min_score, size_t suffix_size);

    std::pair<float, std::string> fill(const std::string_view seq, size_t start, size_t end) const;

    unsigned get_max_size() const;

  private:

    const std::shared_ptr<KmerModel> kmer_model;
    const unsigned max_size;
    const float min_score;
    const size_t suffix_size;
};

}