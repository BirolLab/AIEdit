#pragma once

#include <array>
#include <btllib/nthash.hpp>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "buffer2d.hpp"
#include "edit_list.hpp"
#include "editor.hpp"
#include "kmer_model.hpp"

namespace aiedit {

class ModelInterface
{

  public:

    ModelInterface(const std::string_view seq,
                   size_t start_kmer,
                   size_t end_kmer,
                   unsigned max_mismatches,
                   unsigned max_indels,
                   const std::shared_ptr<KmerModel>& kmer_model);

    Buffer2D get_signature();

    std::tuple<Edit::Type, std::string, float> update(unsigned i_edit);

  private:

    const std::string_view seq;
    const size_t start_kmer, end_kmer;
    const unsigned max_mismatches, max_indels;
    std::shared_ptr<KmerModel> kmer_model;
};

}