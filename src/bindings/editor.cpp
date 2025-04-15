#include "core/editor.hpp"
#include "bind.hpp"

auto editor__iter__(const aiedit::Editor& container)
{
    return pybind11::make_iterator(container.begin(), container.end());
}

REGISTER_BINDING(({
    pybind11::class_<aiedit::Editor>(m, "Editor")
      .def(pybind11::init<const std::string_view, size_t, size_t>())
      .def("substitute", &aiedit::Editor::substitute)
      .def("insert", &aiedit::Editor::insert)
      .def("delete_base", &aiedit::Editor::delete_base)
      .def("skip", &aiedit::Editor::skip)
      .def_property_readonly("size", &aiedit::Editor::get_size)
      .def_property_readonly("position", &aiedit::Editor::get_position)
      .def("__iter__", editor__iter__, pybind11::keep_alive<0, 1>());
}))