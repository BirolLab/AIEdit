#include "core/kmer_model.hpp"
#include "bind.hpp"

auto bf_kmer_model_score(aiedit::BFKmerModel& self, const pybind11::array_t<uint64_t>& hashes)
{
    return self.score(static_cast<uint64_t*>(hashes.request().ptr));
}

auto cbf_kmer_model_score(aiedit::CBFKmerModel& self, const pybind11::array_t<uint64_t>& hashes)
{
    return self.score(static_cast<uint64_t*>(hashes.request().ptr));
}

REGISTER_DEPENDENCY(({
    pybind11::class_<aiedit::KmerModel, std::shared_ptr<aiedit::KmerModel>> kmer_model(m,
                                                                                       "KmerModel");

    pybind11::class_<aiedit::BFKmerModel, std::shared_ptr<aiedit::BFKmerModel>>(m,
                                                                                "BFKmerModel",
                                                                                kmer_model)
      .def(pybind11::init<const std::string&, const std::string&>(),
           pybind11::arg("seeds_bf_path"),
           pybind11::arg("kmers_bf_path"))
      .def("get_seeds", &aiedit::BFKmerModel::get_seeds)
      .def("get_num_hashes", &aiedit::BFKmerModel::get_num_hashes)
      .def("get_kmer_size", &aiedit::BFKmerModel::get_kmer_size)
      .def("get_size", &aiedit::BFKmerModel::get_size)
      .def("get_kmers_fpr", &aiedit::BFKmerModel::get_kmers_fpr)
      .def("get_seeds_fpr", &aiedit::BFKmerModel::get_seeds_fpr)
      .def("score", &bf_kmer_model_score, pybind11::arg("hashes"))
      .def("mean_score", &aiedit::BFKmerModel::mean_score);

    pybind11::class_<aiedit::CBFKmerModel, std::shared_ptr<aiedit::CBFKmerModel>>(m,
                                                                                  "CBFKmerModel",
                                                                                  kmer_model)
      .def(pybind11::init<const std::string&, const std::string&, const std::string&>(),
           pybind11::arg("seeds_bf_path"),
           pybind11::arg("kmers_bf_path"),
           pybind11::arg("hist_path"))
      .def(pybind11::init<const std::string&, const std::string&>(),
           pybind11::arg("seeds_bf_path"),
           pybind11::arg("kmers_bf_path"))
      .def("get_seeds", &aiedit::CBFKmerModel::get_seeds)
      .def("get_num_hashes", &aiedit::CBFKmerModel::get_num_hashes)
      .def("get_kmer_size", &aiedit::CBFKmerModel::get_kmer_size)
      .def("get_size", &aiedit::CBFKmerModel::get_size)
      .def("get_kmers_fpr", &aiedit::CBFKmerModel::get_kmers_fpr)
      .def("get_seeds_fpr", &aiedit::CBFKmerModel::get_seeds_fpr)
      .def("score", &cbf_kmer_model_score, pybind11::arg("hashes"))
      .def("mean_score", &aiedit::CBFKmerModel::mean_score);
}))