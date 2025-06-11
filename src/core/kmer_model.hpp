#pragma once

#include <btllib/bloom_filter.hpp>
#include <btllib/counting_bloom_filter.hpp>
#include <memory>
#include <string>
#include <vector>

namespace aiedit {

class KmerModel
{

  public:

    KmerModel(const std::string& seeds_bf_path);

    virtual float score(const uint64_t* hashes) const = 0;

    virtual unsigned get_num_hashes() const = 0;
    virtual unsigned get_kmer_size() const = 0;
    virtual size_t get_size() const = 0;
    virtual float get_kmers_fpr() const = 0;

    float get_seeds_fpr() const;
    float mean_score(const std::string_view seq) const;
    bool query_seed(const uint64_t* seed_hashes) const;
    const std::vector<std::string>& get_seeds() const;

  protected:

    const std::unique_ptr<btllib::SeedBloomFilter> seeds_bf;
};

class BFKmerModel : public KmerModel
{

  public:

    BFKmerModel(const std::string& seeds_bf_path, const std::string& bf_path);

    float score(const uint64_t* hashes) const;

    unsigned get_num_hashes() const override;
    unsigned get_kmer_size() const override;
    size_t get_size() const override;
    float get_kmers_fpr() const override;

  private:

    const std::unique_ptr<btllib::KmerBloomFilter> bf;
};

class CBFKmerModel : public KmerModel
{

  public:

    CBFKmerModel(const std::string& seeds_bf_path,
                 const std::string& cbf_path,
                 const std::string& hist_path);

    CBFKmerModel(const std::string& seeds_bf_path, const std::string& cbf_path);

    float score(const uint64_t* hashes) const;

    unsigned get_num_hashes() const override;
    unsigned get_kmer_size() const override;
    size_t get_size() const override;
    float get_kmers_fpr() const override;

  private:

    std::vector<float> probs;
    const std::unique_ptr<btllib::KmerCountingBloomFilter8> cbf;
};

}