#include "core/polisher.hpp"
#include "bind.hpp"

#include <torch/script.h>

REGISTER_BINDING(({
    pybind11::class_<aiedit::Polisher>(m, "Polisher")
      .def(pybind11::init<const std::string_view,
                          const std::shared_ptr<aiedit::KmerModel>&,
                          unsigned,
                          float>(),
           pybind11::arg("model_path"),
           pybind11::arg("kmer_model"),
           pybind11::arg("num_threads"),
           pybind11::arg("min_score"))
      .def("polish", &aiedit::Polisher::polish, pybind11::arg("seq"))
      .def("get_max_mismatches", &aiedit::Polisher::get_max_mismatches)
      .def("get_max_indels", &aiedit::Polisher::get_max_indels);
}))