#pragma once

#include <array>
#include <btllib/nthash.hpp>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "buffer2d.hpp"
#include "edit.hpp"
#include "editor.hpp"
#include "kmer_model.hpp"

namespace aiedit {

class ModelInterface
{

  public:

    ModelInterface(const std::string_view seq,
                   size_t start_pos,
                   size_t end_pos,
                   unsigned max_edits,
                   const std::shared_ptr<KmerModel>& kmer_model);

    Buffer2D get_signature();

  private:

    const std::string prefix_kmer;
    Editor editor;
    const unsigned max_edits;
    std::shared_ptr<KmerModel> kmer_model;
};

}