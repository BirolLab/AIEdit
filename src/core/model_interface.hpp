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

    static constexpr unsigned NUM_OUTPUTS = 11;

    ModelInterface(const std::string_view seq,
                   size_t start_kmer_position,
                   size_t end_kmer_position,
                   unsigned max_edits,
                   const std::shared_ptr<KmerModel>& kmer_model);

    std::optional<Edit> operator()(unsigned output_index);

    static Buffer2D encode_seeds(const std::vector<std::string>& seeds);
    Buffer2D get_signature();
    std::array<float, 4> get_next_probs();

    void terminate();
    bool is_terminated() const;
    unsigned get_num_edits_left() const;

  private:

    const std::string prefix_kmer;
    Editor editor;
    unsigned edits_left;
    std::shared_ptr<KmerModel> kmer_model;
};

}