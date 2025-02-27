#pragma once

#include <btllib/nthash.hpp>
#include <memory>
#include <optional>
#include <string>

#include "kmer_model.hpp"

namespace aiedit {

class EditRegionFinder
{
  public:

    EditRegionFinder(std::string_view seq, const std::shared_ptr<KmerModel>& kmer_model);

    std::optional<std::pair<size_t, size_t>> get_next_region();

  private:

    btllib::NtHash hash_fn;
    std::shared_ptr<KmerModel> kmer_model;

    bool next(bool hit);
};

}