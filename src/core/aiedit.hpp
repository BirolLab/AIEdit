#pragma once

#include <string>
#include <vector>

#include "count_probabilities.hpp"
#include "edit.hpp"
#include "pattern_model.hpp"

namespace aiedit {

constexpr auto LOGO = "           _____ ______    _ _ _            \n"
                      "     /\\   |_   _|  ____|  | /_\\ |         \n"
                      "    /  \\    | | | |__   __| | | |_         \n"
                      "   / /\\ \\   | | |  __| / _` | | __|       \n"
                      "  / ____ \\ _| |_| |___| (_| | | |_         \n"
                      " /_/    \\_\\_____|______\\__,_|_|\\__|";

constexpr auto VERSION = "1.0.0";

struct IgnoredPattern {
    size_t pos;
    std::vector<Edit::Type> model_output;
};

struct Results {
    std::vector<Edit> edits;
    std::vector<IgnoredPattern> ignored;
};

class AIEdit
{
  public:

    AIEdit(const std::string& cbf_path,
           const std::string& hist_path,
           const std::string& seeds_path,
           const std::string& model_path,
           unsigned num_threads);

    [[nodiscard]] size_t get_cbf_size() const;
    [[nodiscard]] unsigned get_max_indels() const;
    [[nodiscard]] unsigned get_max_k() const;
    [[nodiscard]] unsigned get_k() const;
    [[nodiscard]] unsigned get_num_seeds() const;

    Results get_edits(const std::string& seq);

  private:

    PatternModel model;
    const CountProbabilities cprobs;
    const unsigned num_threads;

    Results process_chunk(const std::string& seq, size_t start, size_t end);
};

}