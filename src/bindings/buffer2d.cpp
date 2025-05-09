#include "core/buffer2d.hpp"
#include "bind.hpp"

auto buffer2d__setitem__(aiedit::Buffer2D& self, std::pair<size_t, size_t> index, float value)
{
    self.set(index.first, index.second, value);
}

auto buffer2d__getitem__(const aiedit::Buffer2D& self, std::pair<size_t, size_t> index)
{
    return self.get(index.first, index.second);
}

auto buffer2d__buffer(aiedit::Buffer2D& signature)
{
    return pybind11::buffer_info(signature.data(),
                                 sizeof(float),
                                 pybind11::format_descriptor<float>::format(),
                                 2,
                                 {signature.get_num_rows(), signature.get_num_cols()},
                                 {sizeof(float) * signature.get_num_cols(), sizeof(float)});
}

REGISTER_DEPENDENCY(({
    pybind11::class_<aiedit::Buffer2D, std::shared_ptr<aiedit::Buffer2D>>(
      m,
      "Buffer2D",
      pybind11::buffer_protocol())
      .def(pybind11::init<size_t, size_t>())
      .def_property_readonly("num_rows", &aiedit::Buffer2D::get_num_rows)
      .def_property_readonly("num_cols", &aiedit::Buffer2D::get_num_cols)
      .def("__setitem__", &buffer2d__setitem__)
      .def("__getitem__", &buffer2d__getitem__)
      .def_buffer(&buffer2d__buffer);
}))