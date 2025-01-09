#include "pattern_model.hpp"
#include "edit.hpp"
#include "feature_extraction.hpp"

#include <fstream>
#include <stdexcept>

namespace {

const aiedit::Edit::Type edit_types[] = {
  aiedit::Edit::Type::NO_EDIT,
  aiedit::Edit::Type::SUBSTITUION,
  aiedit::Edit::Type::DELETION,
  aiedit::Edit::Type::INSERTION,
};

}

namespace aiedit {

PatternModel::PatternModel(const std::string& model_path, const std::string& seeds_path)
{
    try {
        model = torch::jit::load(model_path);
        model.eval();
        num_seeds = model.attr("num_seeds").toInt();
        max_indels = model.attr("max_indels").toInt();
    } catch (const c10::Error& e) {
        throw std::runtime_error("Failed to load pattern model: " + std::string(e.what()));
    }
    std::ifstream seeds_file(seeds_path);
    if (!seeds_file) {
        throw std::runtime_error("Unable to open seeds file: " + seeds_path);
    }
    std::string seed;
    while (std::getline(seeds_file, seed)) {
        seeds.push_back(seed);
    }
    if (seeds.size() != num_seeds) {
        throw std::runtime_error("Model requires " + std::to_string(num_seeds) +
                                 " spaced seeds (found " + std::to_string(seeds.size()) + ")");
    }
    auto seeds_encoder = model.attr("seeds_encoder").toModule();
    h_seeds = seeds_encoder.forward({encode_seeds(seeds)}).toTensor();
}

std::vector<Edit::Type> PatternModel::get_pattern(const std::string& seq,
                                                  size_t start,
                                                  size_t end,
                                                  const btllib::CountingBloomFilter8& cbf,
                                                  const std::vector<double>& probs)
{
    torch::NoGradGuard no_grad;
    auto probs_encoder = model.attr("probs_encoder").toModule();
    auto x_probs = get_model_input(seq, start, end, seeds, max_indels, cbf, probs);
    auto h_probs = probs_encoder.forward({x_probs, h_seeds}).toTensor();
    auto decoder = model.attr("decoder").toModule();
    auto x_edits = torch::zeros({1, 5});
    torch::Tensor y_decoder = decoder.forward({x_edits, h_probs}).toTensor();
    unsigned tries = max_indels * 2;
    while (y_decoder.index({-1}).argmax().item<int>() != 4 && --tries > 0) {
        x_edits = torch::cat({torch::zeros({1, 5}), y_decoder}, 0);
        y_decoder = decoder.forward({x_edits, h_probs}).toTensor();
    }
    std::vector<Edit::Type> pattern;
    for (unsigned i = 0; i < y_decoder.size(0) - 1; i++) {
        pattern.push_back(edit_types[y_decoder[i].argmax().item<int>()]);
    }
    return pattern;
}

size_t PatternModel::get_k() const { return seeds[0].size(); }

int64_t PatternModel::get_max_indels() const { return max_indels; }

int64_t PatternModel::get_num_seeds() const { return num_seeds; }

}