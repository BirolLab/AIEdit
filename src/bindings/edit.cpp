#include "core/edit.hpp"
#include "bind.hpp"

REGISTER_BINDING(({
    pybind11::enum_<aiedit::Edit::Type>(m, "EditType")
      .value("SUBSTITUTE", aiedit::Edit::Type::SUBSTITUTE)
      .value("INSERT", aiedit::Edit::Type::INSERT)
      .value("DELETE", aiedit::Edit::Type::DELETE);

    pybind11::enum_<aiedit::Edit::Status>(m, "EditStatus")
      .value("PASS", aiedit::Edit::Status::PASS)
      .value("LOW_KMER_SCORE", aiedit::Edit::Status::LOW_KMER_SCORE)
      .value("MODEL_FAIL", aiedit::Edit::Status::MODEL_FAIL);

    pybind11::class_<aiedit::Edit, std::shared_ptr<aiedit::Edit>>(m, "Edit")
      .def(pybind11::init<>())
      .def_readwrite("position", &aiedit::Edit::position)
      .def_readwrite("type", &aiedit::Edit::type)
      .def_readwrite("edited", &aiedit::Edit::edited)
      .def_readwrite("score", &aiedit::Edit::score)
      .def_readwrite("status", &aiedit::Edit::status);
}))