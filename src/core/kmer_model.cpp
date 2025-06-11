#include "kmer_model.hpp"

#include <btllib/nthash.hpp>
#include <fstream>
#include <iterator>
#include <math.h>
#include <sstream>

namespace aiedit {

KmerModel::KmerModel(const std::string& seeds_bf_path)
  : seeds_bf(std::make_unique<btllib::SeedBloomFilter>(seeds_bf_path))
{}

bool KmerModel::query_seed(const uint64_t* seed_hashes) const
{
    return seeds_bf->contains(seed_hashes);
}

float KmerModel::get_seeds_fpr() const { return seeds_bf->get_fpr(); }

float KmerModel::mean_score(const std::string_view seq) const
{
    float q_sum = 0;
    size_t num_kmers = 0;
    btllib::NtHash hash_fn(seq.data(), seq.length(), get_num_hashes(), get_kmer_size());
    while (hash_fn.roll()) {
        const auto kmer_score = score(hash_fn.hashes());
        q_sum += kmer_score > 0.0 ? std::log(kmer_score) : 0.0;
        ++num_kmers;
    }
    return std::exp(q_sum / (float)num_kmers);
}

const std::vector<std::string>& KmerModel::get_seeds() const { return seeds_bf->get_seeds(); };

BFKmerModel::BFKmerModel(const std::string& seeds_bf_path, const std::string& bf_path)
  : KmerModel::KmerModel(seeds_bf_path)
  , bf(std::make_unique<btllib::KmerBloomFilter>(bf_path))
{}

float BFKmerModel::score(const uint64_t* hashes) const { return bf->contains(hashes) ? 1.0 : 0.0; }

unsigned BFKmerModel::get_num_hashes() const { return bf->get_hash_num(); }

unsigned BFKmerModel::get_kmer_size() const { return bf->get_k(); }

size_t BFKmerModel::get_size() const { return bf->get_bytes(); }

float BFKmerModel::get_kmers_fpr() const { return bf->get_fpr(); }

CBFKmerModel::CBFKmerModel(const std::string& seeds_bf_path,
                           const std::string& cbf_path,
                           const std::string& hist_path)
  : KmerModel::KmerModel(seeds_bf_path)
  , cbf(std::make_unique<btllib::KmerCountingBloomFilter8>(cbf_path))
{
    std::ifstream hist_file(hist_path);
    if (!hist_file) {
        throw std::runtime_error("Unable to open k-mer count model file: " + hist_path);
    }
    const auto counter_limit = std::numeric_limits<decltype(cbf->contains(nullptr))>::max();
    probs = std::vector<float>(counter_limit + 1, 0.0);
    std::string line;
    std::getline(hist_file, line);
    auto min_count = counter_limit;
    while (std::getline(hist_file, line)) {
        std::stringstream ss(line);
        std::vector<std::string> row{std::istream_iterator<std::string>(ss),
                                     std::istream_iterator<std::string>()};
        const auto count = std::stoul(row[0]);
        if (count < probs.size()) {
            const auto p0 = std::stod(row[2]), p1 = std::stod(row[3]), p2 = std::stod(row[4]);
            probs[count] = p0 / (p0 + p1 + p2);
            min_count = std::min((unsigned long)min_count, count);
        }
    }
    for (unsigned long i = 0; i < min_count; i++) {
        probs[i] = 1.0;
    }
}

CBFKmerModel::CBFKmerModel(const std::string& seeds_bf_path, const std::string& cbf_path)
  : KmerModel::KmerModel(seeds_bf_path)
  , cbf(std::make_unique<btllib::KmerCountingBloomFilter8>(cbf_path))
{
    const auto counter_limit = std::numeric_limits<decltype(cbf->contains(nullptr))>::max();
    probs = std::vector<float>(counter_limit + 1, 1.0);
    probs[0] = 0.0;
}

float CBFKmerModel::score(const uint64_t* hashes) const { return probs[cbf->contains(hashes)]; }

unsigned CBFKmerModel::get_num_hashes() const { return cbf->get_hash_num(); }

unsigned CBFKmerModel::get_kmer_size() const { return cbf->get_k(); }

size_t CBFKmerModel::get_size() const { return cbf->get_bytes(); }

float CBFKmerModel::get_kmers_fpr() const { return cbf->get_fpr(); }

}
