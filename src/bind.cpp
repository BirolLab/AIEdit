#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "modules/edit.hpp"
#include "modules/editor.hpp"
#include "modules/environment.hpp"
#include "modules/kmer_model.hpp"
#include "modules/signature.hpp"

PYBIND11_MODULE(aiedit, m)
{
    pybind11::class_<aiedit::Edit>(m, "Edit")
      .def_readonly("position", &aiedit::Edit::position)
      .def_readonly("type", &aiedit::Edit::type)
      .def_readonly("before", &aiedit::Edit::before)
      .def_readonly("after", &aiedit::Edit::after)
      .def_static("substitution", &aiedit::Edit::substitution)
      .def_static("insertion", &aiedit::Edit::insertion)
      .def_static("deletion", &aiedit::Edit::deletion);

    pybind11::enum_<aiedit::Edit::Type>(m, "EditType")
      .value("SUBSTITUTE", aiedit::Edit::Type::SUBSTITUTE)
      .value("INSERT", aiedit::Edit::Type::INSERT)
      .value("DELETE", aiedit::Edit::Type::DELETE)
      .value("NONE", aiedit::Edit::Type::NONE);

    pybind11::class_<aiedit::Editor>(m, "Editor")
      .def(pybind11::init<const std::string&, std::shared_ptr<aiedit::KmerModel>>())
      .def("get_next_region", &aiedit::Editor::get_next_region);

    pybind11::class_<aiedit::Environment, std::shared_ptr<aiedit::Environment>>(m, "Environment")
      .def(pybind11::init<const std::string&, size_t, size_t, std::shared_ptr<aiedit::KmerModel>>())
      .def("act", &aiedit::Environment::act)
      .def("get_state", &aiedit::Environment::get_state);

    pybind11::class_<aiedit::Environment::State, std::shared_ptr<aiedit::Environment::State>>(
      m,
      "EnvironmentState")
      .def_readonly("signature", &aiedit::Environment::State::signature);

    pybind11::class_<aiedit::KmerModel, std::shared_ptr<aiedit::KmerModel>>(m, "KmerModel")
      .def(pybind11::init<const std::string&, const std::string&, const std::string&>())
      .def("get_num_hashes", &aiedit::KmerModel::get_num_hashes)
      .def("get_kmer_size", &aiedit::KmerModel::get_kmer_size)
      .def("score", &aiedit::KmerModel::score);

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