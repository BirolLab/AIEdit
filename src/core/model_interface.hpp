#pragma once

#include <array>
#include <btllib/nthash.hpp>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "buffer2d.hpp"
#include "editor.hpp"
#include "kmer_model.hpp"

namespace aiedit {

class ModelInterface
{

  public:

    ModelInterface(const std::string_view seq,
                   size_t start_pos,
                   size_t end_pos,
                   unsigned max_indels,
                   const std::shared_ptr<KmerModel>& kmer_model);

    Buffer2D get_signature();

    std::optional<std::string>
    update(float indel_prob, std::vector<float> mismatches, unsigned indels);

  private:

    const std::string prefix_kmer;
    Editor editor;
    const unsigned max_indels;
    std::shared_ptr<KmerModel> kmer_model;
};

}