#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "core/buffer2d.hpp"
#include "core/edit.hpp"
#include "core/edit_region_finder.hpp"
#include "core/editor.hpp"
#include "core/kmer_model.hpp"
#include "core/model_interface.hpp"
#include "core/utils.hpp"

PYBIND11_MODULE(core, m)
{
    pybind11::enum_<aiedit::Edit::Type>(m, "EditType")
      .value("SUBSTITUTE", aiedit::Edit::Type::SUBSTITUTE)
      .value("INSERT", aiedit::Edit::Type::INSERT)
      .value("DELETE", aiedit::Edit::Type::DELETE);

    pybind11::class_<aiedit::Edit>(m, "Edit")
      .def_readonly("position", &aiedit::Edit::position)
      .def_readonly("type", &aiedit::Edit::type)
      .def_readonly("new_base", &aiedit::Edit::new_base)
      .def_static("substitution", &aiedit::Edit::substitution)
      .def_static("insertion", &aiedit::Edit::insertion)
      .def_static("deletion", &aiedit::Edit::deletion);

    pybind11::class_<aiedit::KmerModel, std::shared_ptr<aiedit::KmerModel>> kmer_model(m,
                                                                                       "KmerModel");

    pybind11::class_<aiedit::BFKmerModel, std::shared_ptr<aiedit::BFKmerModel>>(m,
                                                                                "BFKmerModel",
                                                                                kmer_model)
      .def(pybind11::init<const std::string&>())
      .def("get_seeds", &aiedit::BFKmerModel::get_seeds)
      .def("get_num_hashes", &aiedit::BFKmerModel::get_num_hashes)
      .def("get_kmer_size", &aiedit::BFKmerModel::get_kmer_size)
      .def("score", [](aiedit::BFKmerModel& self, const pybind11::array_t<uint64_t>& hashes) {
          return self.score(static_cast<uint64_t*>(hashes.request().ptr));
      });

    pybind11::class_<aiedit::CBFKmerModel, std::shared_ptr<aiedit::CBFKmerModel>>(m,
                                                                                  "CBFKmerModel",
                                                                                  kmer_model)
      .def(pybind11::init<const std::string&, const std::vector<std::string>>())
      .def("get_seeds", &aiedit::CBFKmerModel::get_seeds)
      .def("get_num_hashes", &aiedit::CBFKmerModel::get_num_hashes)
      .def("get_kmer_size", &aiedit::CBFKmerModel::get_kmer_size)
      .def("score", [](aiedit::CBFKmerModel& self, const pybind11::array_t<uint64_t>& hashes) {
          return self.score(static_cast<uint64_t*>(hashes.request().ptr));
      });

    pybind11::class_<aiedit::EditRegionFinder, std::shared_ptr<aiedit::EditRegionFinder>>(
      m,
      "EditRegionFinder")
      .def(
        pybind11::init<const std::string_view, const std::shared_ptr<aiedit::KmerModel>&, float>())
      .def(
        "__iter__",
        [](aiedit::EditRegionFinder& container) {
            return pybind11::make_iterator(container.begin(), container.end());
        },
        pybind11::keep_alive<0, 1>());

    pybind11::class_<aiedit::Editor>(m, "Editor")
      .def(pybind11::init<const std::string_view, size_t, size_t>())
      .def("substitute", &aiedit::Editor::substitute)
      .def("insert", &aiedit::Editor::insert)
      .def("delete_base", &aiedit::Editor::delete_base)
      .def("skip", &aiedit::Editor::skip)
      .def_property_readonly("size", &aiedit::Editor::get_size)
      .def_property_readonly("position", &aiedit::Editor::get_position)
      .def(
        "__iter__",
        [](const aiedit::Editor& container) {
            return pybind11::make_iterator(container.begin(), container.end());
        },
        pybind11::keep_alive<0, 1>());

    pybind11::class_<aiedit::Buffer2D, std::shared_ptr<aiedit::Buffer2D>>(
      m,
      "Buffer2D",
      pybind11::buffer_protocol())
      .def(pybind11::init<size_t, size_t>())
      .def_property_readonly("num_rows", &aiedit::Buffer2D::get_num_rows)
      .def_property_readonly("num_cols", &aiedit::Buffer2D::get_num_cols)
      .def("__setitem__",
           [](aiedit::Buffer2D& self, std::pair<size_t, size_t> index, float value) {
               self.set(index.first, index.second, value);
           })
      .def("__getitem__",
           [](const aiedit::Buffer2D& self, std::pair<size_t, size_t> index) {
               return self.get(index.first, index.second);
           })
      .def_buffer([](aiedit::Buffer2D& signature) -> pybind11::buffer_info {
          return pybind11::buffer_info(signature.data(),
                                       sizeof(float),
                                       pybind11::format_descriptor<float>::format(),
                                       2,
                                       {signature.get_num_rows(), signature.get_num_cols()},
                                       {sizeof(float) * signature.get_num_cols(), sizeof(float)});
      });

    pybind11::class_<aiedit::ModelInterface, std::shared_ptr<aiedit::ModelInterface>>(
      m,
      "ModelInterface")
      .def(pybind11::init<const std::string_view,
                          size_t,
                          size_t,
                          unsigned,
                          const std::shared_ptr<aiedit::KmerModel>&>())
      .def("get_signature", &aiedit::ModelInterface::get_signature)
      .def("update", &aiedit::ModelInterface::update);

    m.def("apply_edits", &aiedit::apply_edits);
}