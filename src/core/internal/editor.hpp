#pragma once

#include <torch/nn/module.h>

namespace aiedit::internal {

class Editor
{

  public:

    Editor(const std::string& path);

    void save(const std::string& path);

    torch::Tensor get_indels(const torch::Tensor& x);

  private:

    torch::nn::Module indel_model;
    torch::nn::Module subs_model;
};

}