#include "core/model_interface.hpp"
#include "bind.hpp"

REGISTER_BINDING(({
    pybind11::class_<aiedit::ModelInterface, std::shared_ptr<aiedit::ModelInterface>>(
      m,
      "ModelInterface")
      .def(pybind11::init<const std::string_view,
                          size_t,
                          size_t,
                          unsigned,
                          unsigned,
                          const std::shared_ptr<aiedit::KmerModel>&>(),
           pybind11::arg("seq"),
           pybind11::arg("start_kmer"),
           pybind11::arg("end_kmer"),
           pybind11::arg("max_mismatches"),
           pybind11::arg("max_indels"),
           pybind11::arg("kmer_model"))
      .def("get_signature", &aiedit::ModelInterface::get_signature)
      .def("update", &aiedit::ModelInterface::update);
}))