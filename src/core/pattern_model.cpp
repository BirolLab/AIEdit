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
        max_k = model.attr("max_k").toInt();
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
    if (seeds[0].size() > max_k) {
        throw std::runtime_error("Maximum k supported by model is " + std::to_string(max_k));
    }
    x_seeds = encode_seeds(seeds, max_k);
}

std::vector<Edit::Type> PatternModel::get_pattern(const std::string& seq,
                                                  size_t start,
                                                  size_t end,
                                                  const btllib::CountingBloomFilter8& cbf,
                                                  const std::vector<double>& probs)
{
    torch::NoGradGuard no_grad;
    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(get_model_input(seq, start, end, seeds, max_indels, cbf, probs));
    inputs.push_back(x_seeds);
    inputs.push_back(torch::zeros({1, 5}));
    torch::Tensor output = model.forward(inputs).toTensor();
    unsigned tries = max_indels;
    while (output.index({-1}).argmax().item<int>() != 4 && --tries > 0) {
        inputs[2] = torch::cat({torch::zeros({1, 5}), output}, 0);
        output = model.forward(inputs).toTensor();
    }
    std::vector<Edit::Type> pattern;
    for (unsigned i = 0; i < output.size(0) - 1; i++) {
        pattern.push_back(edit_types[output[i].argmax().item<int>()]);
    }
    return pattern;
}

size_t PatternModel::get_k() const { return seeds[0].size(); }

int64_t PatternModel::get_max_indels() const { return max_indels; }

int64_t PatternModel::get_max_k() const { return max_k; }

int64_t PatternModel::get_num_seeds() const { return num_seeds; }

}