#include "signature.hpp"

#include <pybind11/pybind11.h>

namespace aiedit {

Signature::Signature(size_t length, size_t num_seeds)
  : length(length)
  , num_seeds(num_seeds)
  , data_ptr(std::make_unique<float[]>(length * num_seeds))
{}

void Signature::set(size_t position, size_t i_seed, float value)
{
    data_ptr[position * num_seeds + i_seed] = value;
}

float Signature::get(size_t position, size_t i_seed) const
{
    return data_ptr[position * length + i_seed];
}

float* Signature::data() const { return data_ptr.get(); }

size_t Signature::get_length() const { return length; }

size_t Signature::get_num_seeds() const { return num_seeds; }

}

void bind_signature(pybind11::module_& m)
{
    pybind11::class_<aiedit::Signature, std::shared_ptr<aiedit::Signature>>(
      m,
      "Signature",
      pybind11::buffer_protocol())
      .def_buffer([](aiedit::Signature& signature) -> pybind11::buffer_info {
          return pybind11::buffer_info(signature.data(),
                                       sizeof(float),
                                       pybind11::format_descriptor<float>::format(),
                                       2,
                                       {signature.get_length(), signature.get_num_seeds()},
                                       {sizeof(float) * signature.get_num_seeds(), sizeof(float)});
      });
}