#include "feature_extraction.hpp"

#include <torch/extension.h>

PYBIND11_MODULE(aiedit_torch_extensions, m) { m.def("get_model_input", &aiedit::get_model_input_wrapper); }