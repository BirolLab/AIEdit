#include "core/seed_generator.hpp"
#include "bind.hpp"

REGISTER_BINDING(({
    pybind11::class_<aiedit::SeedGenerator>(m, "SeedGenerator")
      .def(pybind11::init<unsigned, unsigned, float>(),
           pybind11::arg("population_size"),
           pybind11::arg("max_generations"),
           pybind11::arg("mutation_probability"))
      .def("generate",
           &aiedit::SeedGenerator::generate,
           pybind11::arg("num_seeds"),
           pybind11::arg("kmer_size"),
           pybind11::arg("max_mismatches"),
           pybind11::arg("max_indels"));
}))