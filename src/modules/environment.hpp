#pragma once

#include <array>
#include <btllib/nthash.hpp>
#include <cstddef>
#include <deque>
#include <memory>
#include <string>
#include <tuple>

#include "edit.hpp"
#include "kmer_model.hpp"
#include "region_editor.hpp"
#include "signature.hpp"

namespace aiedit {

class Environment
{

  public:

    Environment(const std::string_view seq,
                size_t start,
                size_t end,
                unsigned max_edits,
                std::shared_ptr<KmerModel> kmer_model);

    void act(Edit::Type edit_type, char new_base);

    Signature get_signature();
    std::array<float, 4> get_next_probs();

    void terminate();

    bool is_terminated() const;

  private:

    const std::string prefix;
    RegionEditor editor;
    unsigned max_edits;
    std::shared_ptr<KmerModel> kmer_model;
    std::vector<Edit> edits;
};

}