#include "core/feature_extraction.hpp"

#include <torch/extension.h>

PYBIND11_MODULE(aiedit_torch_extensions, m)
{
    m.def("positional_encoding", &aiedit::positional_encoding);
    m.def("encode_seeds", &aiedit::encode_seeds);
    m.def("get_model_input", &aiedit::get_model_input_wrapper);
}