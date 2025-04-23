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

    class Iterator;

    EditRegionFinder(const std::string_view seq,
                     const std::shared_ptr<KmerModel>& kmer_model,
                     float hit_threshold,
                     unsigned max_length);

    EditRegionFinder::Iterator begin();
    EditRegionFinder::Iterator end();

  private:

    btllib::NtHash hash_fn;
    std::shared_ptr<KmerModel> kmer_model;
    const float hit_threshold;
    const unsigned max_length;

    std::optional<std::pair<size_t, size_t>> get_next_region();
    bool next(bool hit);
};

class EditRegionFinder::Iterator
{

  public:

    std::pair<size_t, size_t> operator*() const;
    Iterator& operator++();
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

  private:

    EditRegionFinder& erf;
    std::pair<size_t, size_t> current;

    Iterator(EditRegionFinder& erf, bool done);

    friend class EditRegionFinder;
};

}