#include "core/editor.hpp"
#include "bind.hpp"

auto editor__iter__(const aiedit::Editor& container)
{
    return pybind11::make_iterator(container.begin(), container.end());
}

REGISTER_BINDING(({
    pybind11::class_<aiedit::Editor>(m, "Editor")
      .def(pybind11::init<const std::string_view, size_t, size_t>(),
           pybind11::arg("seq"),
           pybind11::arg("start_pos"),
           pybind11::arg("end_pos"))
      .def("substitute", &aiedit::Editor::substitute, pybind11::arg("new_base"))
      .def("insert", &aiedit::Editor::insert, pybind11::arg("base"))
      .def("delete_base", &aiedit::Editor::delete_base)
      .def("skip", &aiedit::Editor::skip)
      .def("get_num_remaining", &aiedit::Editor::get_num_remaining)
      .def("get_current", &aiedit::Editor::get_current)
      .def("get_size", &aiedit::Editor::get_size)
      .def("__iter__", editor__iter__, pybind11::keep_alive<0, 1>());
}))