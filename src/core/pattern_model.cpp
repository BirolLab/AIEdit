#include "pattern_model.hpp"
#include "feature_extraction.hpp"

#include <stdexcept>
#include <string>

namespace {
const std::string edit_chars = ".MID|";
}

namespace aiedit {

PatternModel::PatternModel(const std::string& model_path, const std::vector<std::string>& seeds)
  : model(torch::jit::load(model_path))
  , seeds(seeds)
  , x_seeds(encode_seeds(seeds, model.attr("max_k").toInt()))
{
    model.eval();
    const auto num_seeds = model.attr("num_seeds").toInt();
    const auto max_k = model.attr("max_k").toInt();
    if (seeds.size() != num_seeds) {
        throw std::runtime_error("Model requires " + std::to_string(num_seeds) + " spaced seeds");
    }
    if (seeds[0].size() > max_k) {
        throw std::runtime_error("Maximum k supported by model is " + std::to_string(max_k));
    }
}

std::string PatternModel::get_pattern(const std::string& seq,
                                      size_t start,
                                      size_t end,
                                      const btllib::CountingBloomFilter8& cbf,
                                      const std::vector<double>& probs)
{
    torch::NoGradGuard no_grad;
    const auto max_indels = model.attr("max_indels").toInt();
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
    std::string pattern;
    for (unsigned i = 0; i < output.size(0) - 1; i++) {
        pattern.push_back(edit_chars[output[i].argmax().item<int>()]);
    }
    return pattern;
}

}  // namespace aiedit