#include "core/edit_region_finder.hpp"
#include "bind.hpp"

auto edit_region_finder__iter__(aiedit::EditRegionFinder& container)
{
    return pybind11::make_iterator(container.begin(), container.end());
}

REGISTER_BINDING(({
    pybind11::class_<aiedit::EditRegionFinder, std::shared_ptr<aiedit::EditRegionFinder>>(
      m,
      "EditRegionFinder")
      .def(pybind11::init<const std::string_view,
                          const std::shared_ptr<aiedit::KmerModel>&,
                          float,
                          unsigned>())
      .def("__iter__", &edit_region_finder__iter__, pybind11::keep_alive<0, 1>());
}))