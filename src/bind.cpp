#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "core/buffer2d.hpp"
#include "core/edit.hpp"
#include "core/edit_region_finder.hpp"
#include "core/editor.hpp"
#include "core/kmer_model.hpp"
#include "core/model_interface.hpp"

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
      .def(pybind11::init<const std::string_view, const std::shared_ptr<aiedit::KmerModel>&>())
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
      .def_property_readonly_static(
        "NUM_OUTPUTS",
        [](pybind11::object) { return aiedit::ModelInterface::NUM_OUTPUTS; })
      .def("__call__",
           [](aiedit::ModelInterface& self, unsigned output_index) { return self(output_index); })
      .def("terminate", &aiedit::ModelInterface::terminate)
      .def("is_terminated", &aiedit::ModelInterface::is_terminated)
      .def_property_readonly("num_edits_left", &aiedit::ModelInterface::get_num_edits_left)
      .def("get_next_probs", &aiedit::ModelInterface::get_next_probs)
      .def_static("encode_seeds", &aiedit::ModelInterface::encode_seeds)
      .def("get_signature", &aiedit::ModelInterface::get_signature);
}