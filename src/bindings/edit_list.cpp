#include "core/edit_list.hpp"
#include "bind.hpp"

#include <memory>

auto edits_list__iter__(const aiedit::EditList& container)
{
    return pybind11::make_iterator(container.begin(), container.end());
}

REGISTER_BINDING(({
    pybind11::class_<aiedit::EditList, std::shared_ptr<aiedit::EditList>>(m, "EditList")
      .def(pybind11::init<>())
      .def("add", &aiedit::EditList::add)
      .def("sort", &aiedit::EditList::sort)
      .def("apply", &aiedit::EditList::apply)
      .def("get_num_passed", &aiedit::EditList::get_num_passed)
      .def("__len__", &aiedit::EditList::size)
      .def("__iter__", edits_list__iter__, pybind11::keep_alive<0, 1>());
}))