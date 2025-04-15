#include "bind.hpp"
#include "core/edit.hpp"

REGISTER_BINDING(({
    pybind11::enum_<aiedit::Edit::Type>(m, "EditType")
      .value("SUBSTITUTE", aiedit::Edit::Type::SUBSTITUTE)
      .value("INSERT", aiedit::Edit::Type::INSERT)
      .value("DELETE", aiedit::Edit::Type::DELETE);

      pybind11::class_<aiedit::Edit>(m, "Edit")
      .def_readonly("position", &aiedit::Edit::position)
      .def_readonly("type", &aiedit::Edit::type)
      .def_readonly("new_base", &aiedit::Edit::new_base)
      .def_static("substitution", &aiedit::Edit::substitution)
      .def_static("insertion", &aiedit::Edit::insertion)
      .def_static("deletion", &aiedit::Edit::deletion);
}))