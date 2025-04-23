#include "kmer_model.hpp"
#include <fstream>
#include <iterator>
#include <sstream>

namespace aiedit {

BFKmerModel::BFKmerModel(const std::string& bf_path)
  : bf(std::make_shared<btllib::SeedBloomFilter>(bf_path))
{}

float BFKmerModel::score(const uint64_t* hashes) const { return bf->contains(hashes) ? 1.0 : 0.0; }

unsigned BFKmerModel::get_num_hashes() const { return bf->get_hash_num_per_seed(); }

unsigned BFKmerModel::get_kmer_size() const { return bf->get_k(); }

size_t BFKmerModel::get_size() const { return bf->get_bytes(); }

const std::vector<std::string>& BFKmerModel::get_seeds() const { return bf->get_seeds(); };

CBFKmerModel::CBFKmerModel(const std::string& cbf_path,
                           const std::string& hist_path,
                           const std::vector<std::string> seeds)
  : cbf(std::make_shared<btllib::CountingBloomFilter8>(cbf_path))
  , seeds(seeds)
{}

float CBFKmerModel::score(const uint64_t* hashes) const
{
    return cbf->contains(hashes) > 0 ? 1.0 : 0.0;  // TODO
}

unsigned CBFKmerModel::get_num_hashes() const { return cbf->get_hash_num(); }

unsigned CBFKmerModel::get_kmer_size() const { return seeds[0].size(); }

size_t CBFKmerModel::get_size() const { return cbf->get_bytes(); }

const std::vector<std::string>& CBFKmerModel::get_seeds() const { return seeds; };

}
