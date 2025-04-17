#include "core/model_interface.hpp"
#include "bind.hpp"

auto update_model_interface(aiedit::ModelInterface& self,
                            const std::vector<uintptr_t>& outputs,
                            const std::vector<long>& sizes)
{
    std::vector<float*> outputs_ptrs;
    for (const auto data_ptr : outputs) {
        outputs_ptrs.push_back(reinterpret_cast<float*>(data_ptr));
    }
    return self.update(outputs_ptrs, sizes);
}

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
      .def("update", &update_model_interface);
}))