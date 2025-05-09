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
                   unsigned max_indels,
                   const std::shared_ptr<KmerModel>& kmer_model);

    Buffer2D get_signature();

    std::tuple<Edit::Type, std::string, float> update(const std::vector<float*>& outputs,
                                                      const std::vector<long>& sizes);

  private:

    const std::string_view seq;
    const size_t start_kmer;
    Editor editor;
    const unsigned max_indels;
    std::shared_ptr<KmerModel> kmer_model;
};

}