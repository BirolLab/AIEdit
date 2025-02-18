#include "editor.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace {

inline bool next(btllib::NtHash& hash_fn,
                 std::shared_ptr<aiedit::KmerModel> kmer_model,
                 double threshold,
                 bool hit)
{
    while (hash_fn.roll()) {
        if ((kmer_model->score(hash_fn.hashes()) < threshold) == hit) {
            return true;
        }
    }
    return false;
}

}

namespace aiedit {

Editor::Editor(const std::string& seq, std::shared_ptr<KmerModel> kmer_model)
  : hash_fn(seq, kmer_model->get_num_hashes(), kmer_model->get_kmer_size())
  , kmer_model(kmer_model)
{
    next(hash_fn, kmer_model, 0.5, true);
}

std::optional<std::pair<size_t, size_t>> Editor::get_next_region()
{
    if (!next(hash_fn, kmer_model, 0.5, false)) {
        return {};
    }
    size_t start_pos = hash_fn.get_pos();
    if (!next(hash_fn, kmer_model, 0.5, true)) {
        return {};
    }
    size_t end_pos = hash_fn.get_pos();
    return std::make_pair(start_pos, end_pos);
}

}

void bind_editor(pybind11::module_& m)
{
    pybind11::class_<aiedit::Editor>(m, "Editor")
      .def(pybind11::init<const std::string&, std::shared_ptr<aiedit::KmerModel>>())
      .def("get_next_region", &aiedit::Editor::get_next_region);
}