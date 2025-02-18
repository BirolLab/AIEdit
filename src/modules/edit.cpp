#include "edit.hpp"

#include <pybind11/pybind11.h>

namespace aiedit {

Edit::Edit(size_t position, Type type, char before, char after)
  : position(position)
  , type(type)
  , before(before)
  , after(after)
{}

Edit Edit::substitution(size_t position, char before, char after)
{
    return Edit(position, Edit::Type::SUBSTITUTE, before, after);
}

Edit Edit::insertion(size_t position, char base)
{
    return Edit(position, Edit::Type::INSERT, Edit::NO_BASE, base);
}

Edit Edit::deletion(size_t position, char base)
{
    return Edit(position, Edit::Type::DELETE, base, Edit::NO_BASE);
}

}

void bind_edit_type(pybind11::module_& m)
{
    pybind11::enum_<aiedit::Edit::Type>(m, "EditType")
      .value("SUBSTITUTE", aiedit::Edit::Type::SUBSTITUTE)
      .value("INSERT", aiedit::Edit::Type::INSERT)
      .value("DELETE", aiedit::Edit::Type::DELETE)
      .value("NONE", aiedit::Edit::Type::NONE);
}

void bind_edit(pybind11::module_& m)
{
    pybind11::class_<aiedit::Edit>(m, "Edit")
      .def_readonly("position", &aiedit::Edit::position)
      .def_readonly("type", &aiedit::Edit::type)
      .def_readonly("before", &aiedit::Edit::before)
      .def_readonly("after", &aiedit::Edit::after)
      .def_static("substitution", &aiedit::Edit::substitution)
      .def_static("insertion", &aiedit::Edit::insertion)
      .def_static("deletion", &aiedit::Edit::deletion);
}
