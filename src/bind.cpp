#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "modules/edit.hpp"
#include "modules/edit_region_finder.hpp"
#include "modules/environment.hpp"
#include "modules/kmer_model.hpp"
#include "modules/signature.hpp"

PYBIND11_MODULE(aiedit, m)
{
    pybind11::class_<aiedit::Edit>(m, "Edit")
      .def_readonly("position", &aiedit::Edit::position)
      .def_readonly("type", &aiedit::Edit::type)
      .def_readonly("new_base", &aiedit::Edit::new_base)
      .def_static("substitution", &aiedit::Edit::substitution)
      .def_static("insertion", &aiedit::Edit::insertion)
      .def_static("deletion", &aiedit::Edit::deletion);

    pybind11::enum_<aiedit::Edit::Type>(m, "EditType")
      .value("SUBSTITUTE", aiedit::Edit::Type::SUBSTITUTE)
      .value("INSERT", aiedit::Edit::Type::INSERT)
      .value("DELETE", aiedit::Edit::Type::DELETE);

    pybind11::class_<aiedit::KmerModel, std::shared_ptr<aiedit::KmerModel>>(m, "KmerModel")
      .def(pybind11::init<const std::string&, const std::string&, const std::string&, float>())
      .def_readonly("seeds", &aiedit::KmerModel::seeds)
      .def_property_readonly("num_hashes", &aiedit::KmerModel::get_num_hashes)
      .def_property_readonly("kmer_size", &aiedit::KmerModel::get_kmer_size)
      .def("is_hit", [](aiedit::KmerModel& self, const pybind11::array_t<uint64_t>& hashes) {
          return self.is_hit(static_cast<uint64_t*>(hashes.request().ptr));
      });

    pybind11::class_<aiedit::EditRegionFinder, std::shared_ptr<aiedit::EditRegionFinder>>(
      m,
      "EditRegionFinder")
      .def(pybind11::init<std::string_view, const std::shared_ptr<aiedit::KmerModel>&>())
      .def("get_next_region", &aiedit::EditRegionFinder::get_next_region);

    pybind11::class_<aiedit::Environment, std::shared_ptr<aiedit::Environment>>(m, "Environment")
      .def(
        pybind11::
          init<const std::string&, size_t, size_t, unsigned, std::shared_ptr<aiedit::KmerModel>>())
      .def("act", &aiedit::Environment::act)
      .def("get_state", &aiedit::Environment::get_state);

    pybind11::class_<aiedit::Environment::State, std::shared_ptr<aiedit::Environment::State>>(
      m,
      "EnvironmentState")
      .def_readonly("signature", &aiedit::Environment::State::signature)
      .def_readonly("next_probs", &aiedit::Environment::State::next_probs);

    pybind11::class_<aiedit::Signature, std::shared_ptr<aiedit::Signature>>(
      m,
      "Signature",
      pybind11::buffer_protocol())
      .def(pybind11::init<size_t, size_t>())
      .def_property_readonly("length", &aiedit::Signature::get_length)
      .def_property_readonly("num_seeds", &aiedit::Signature::get_num_seeds)
      .def("__setitem__",
           [](aiedit::Signature& self, std::pair<size_t, size_t> index, float value) {
               self.set(index.first, index.second, value);
           })
      .def("__getitem__",
           [](const aiedit::Signature& self, std::pair<size_t, size_t> index) {
               return self.get(index.first, index.second);
           })
      .def(pybind11::init<size_t, size_t>())
      .def_buffer([](aiedit::Signature& signature) -> pybind11::buffer_info {
          return pybind11::buffer_info(signature.data(),
                                       sizeof(float),
                                       pybind11::format_descriptor<float>::format(),
                                       2,
                                       {signature.get_length(), signature.get_num_seeds()},
                                       {sizeof(float) * signature.get_num_seeds(), sizeof(float)});
      });
}