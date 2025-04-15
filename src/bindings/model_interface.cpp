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
                          const std::shared_ptr<aiedit::KmerModel>&>())
      .def("get_signature", &aiedit::ModelInterface::get_signature)
      .def("update", &aiedit::ModelInterface::update);
}))