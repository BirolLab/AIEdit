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

    virtual float score(const uint64_t* hashes) const = 0;

    virtual unsigned get_num_hashes() const = 0;
    virtual unsigned get_kmer_size() const = 0;
    virtual const std::vector<std::string>& get_seeds() const = 0;
};

class BFKmerModel : public KmerModel
{

  public:

    BFKmerModel(const std::string& bf_path);

    float score(const uint64_t* hashes) const;

    unsigned get_num_hashes() const;
    unsigned get_kmer_size() const;
    const std::vector<std::string>& get_seeds() const;

  private:

    const std::shared_ptr<btllib::SeedBloomFilter> bf;
};

class CBFKmerModel : public KmerModel
{

  public:

    CBFKmerModel(const std::string& cbf_path, const std::vector<std::string> seeds);

    float score(const uint64_t* hashes) const;

    unsigned get_num_hashes() const override;
    unsigned get_kmer_size() const override;
    const std::vector<std::string>& get_seeds() const;

  private:

    const std::vector<std::string> seeds;
    const std::shared_ptr<btllib::CountingBloomFilter8> cbf;
};

}