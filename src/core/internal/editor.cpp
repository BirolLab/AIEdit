#include "editor.hpp"

#include <vector>

namespace {

struct IndelModel : torch::nn::Module {

    IndelModel(unsigned num_seeds) {}

    torch::Tensor forward(const torch::Tensor& x) { return x; }
};

struct SubsModel : torch::nn::Module {};

}

namespace aiedit::internal {

Editor::Editor(const std::string& path) {}

void Editor::save(const std::string& path) {}

torch::Tensor Editor::get_indels(const torch::Tensor& x)
{
    
}

}