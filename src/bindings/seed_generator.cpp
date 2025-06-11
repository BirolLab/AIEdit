#include "core/seed_generator.hpp"
#include "bind.hpp"

REGISTER_BINDING(({
    pybind11::class_<aiedit::SeedGenerator>(m, "SeedGenerator")
      .def(pybind11::init<unsigned, unsigned, float, std::optional<unsigned>>(),
           pybind11::arg("population_size"),
           pybind11::arg("max_generations"),
           pybind11::arg("mutation_probability"),
           pybind11::arg("random_seed") = std::nullopt)
      .def("generate",
           &aiedit::SeedGenerator::generate,
           pybind11::arg("num_seeds"),
           pybind11::arg("kmer_size"),
           pybind11::arg("max_mismatches"),
           pybind11::arg("max_indels"));
}))