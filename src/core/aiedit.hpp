#pragma once

#include <ATen/Tensor.h>
#include <btllib/counting_bloom_filter.hpp>
#include <string>
#include <torch/script.h>
#include <vector>

#include "count_probabilities.hpp"
#include "pattern_model.hpp"

namespace aiedit {

constexpr auto LOGO = "           _____ ______    _ _ _            \n"
                      "     /\\   |_   _|  ____|  | /_\\ |         \n"
                      "    /  \\    | | | |__   __| | | |_         \n"
                      "   / /\\ \\   | | |  __| / _` | | __|       \n"
                      "  / ____ \\ _| |_| |___| (_| | | |_         \n"
                      " /_/    \\_\\_____|______\\__,_|_|\\__|";

constexpr auto VERSION = "1.0.0";

class AIEdit
{
  public:

    AIEdit(const std::string& cbf_path,
           const std::string& hist_path,
           const std::string& seeds_path,
           const std::string& model_path);

    [[nodiscard]] size_t get_cbf_size() const;
    [[nodiscard]] unsigned get_max_indels() const;
    [[nodiscard]] unsigned get_max_k() const;
    [[nodiscard]] unsigned get_k() const;
    [[nodiscard]] unsigned get_num_seeds() const;

    void get_edits(const std::string& seq, size_t start, size_t end);

  private:

    const CountProbabilities cprobs;
    const PatternModel model;
    std::vector<std::string> seeds;
};

}  // namespace aiedit