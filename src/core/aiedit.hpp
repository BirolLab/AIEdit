#pragma once

#include <btllib/counting_bloom_filter.hpp>
#include <string>
#include <vector>

#include "internal/editor.hpp"

namespace aiedit {

constexpr auto LOGO = "           _____ ______    _ _ _            \n"
                      "     /\\   |_   _|  ____|  | /_\\ |         \n"
                      "    /  \\    | | | |__   __| | | |_         \n"
                      "   / /\\ \\   | | |  __| / _` | | __|       \n"
                      "  / ____ \\ _| |_| |___| (_| | | |_         \n"
                      " /_/    \\_\\_____|______\\__,_|_|\\__|";

constexpr auto VERSION = "1.0.0";

struct Edit {
    const size_t pos;
    const std::string before;
    const int indel;
    const std::string subs;
    const double score;
};

struct TrainingStep {
    const double loss;
    const double reward;
};

class AIEdit
{
  public:

    AIEdit(const std::string& cbf_path,
           const std::string& hist_path,
           const std::string& seeds_path,
           const std::string& model_path,
           unsigned max_edits);

    std::vector<TrainingStep> train(const std::string& seq);

    std::vector<Edit> get_edits(const std::string& seq, size_t start, size_t end);

  private:

    const btllib::CountingBloomFilter8 cbf;
    const std::vector<double> probs;
    const std::vector<std::string> seeds;
    internal::Editor editor;
    unsigned max_edits;
};

}