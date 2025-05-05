#include "core/polisher.hpp"
#include "bind.hpp"

#include <torch/script.h>

REGISTER_BINDING(({
    pybind11::class_<aiedit::Polisher>(m, "Polisher")
      .def(pybind11::init<const std::string_view,
                          const std::shared_ptr<aiedit::KmerModel>&,
                          unsigned,
                          float>())
      .def("polish", &aiedit::Polisher::polish)
      .def("get_max_mismatches", &aiedit::Polisher::get_max_mismatches)
      .def("get_max_indels", &aiedit::Polisher::get_max_indels);
}))