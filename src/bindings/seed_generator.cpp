#include "core/seed_generator.hpp"
#include "bind.hpp"

REGISTER_BINDING(({
    pybind11::class_<aiedit::SeedGenerator>(m, "SeedGenerator")
      .def(pybind11::init<unsigned, unsigned, float>())
      .def("generate", &aiedit::SeedGenerator::generate);
}))