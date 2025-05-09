#include "core/edit_list.hpp"
#include "bind.hpp"

#include <memory>

auto edits_list__iter__(const aiedit::EditList& container)
{
    return pybind11::make_iterator(container.begin(), container.end());
}

REGISTER_DEPENDENCY(({
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

    pybind11::class_<aiedit::EditList, std::shared_ptr<aiedit::EditList>>(m, "EditList")
      .def(pybind11::init<>())
      .def("add", &aiedit::EditList::add, pybind11::arg("edit"))
      .def("sort", &aiedit::EditList::sort)
      .def("apply", &aiedit::EditList::apply, pybind11::arg("seq"))
      .def("get_num_passed", &aiedit::EditList::get_num_passed)
      .def("__len__", &aiedit::EditList::size)
      .def("__iter__", edits_list__iter__, pybind11::keep_alive<0, 1>());
}))