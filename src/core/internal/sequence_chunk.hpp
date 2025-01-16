#pragma once

#include <btllib/nthash.hpp>
#include <cstddef>
#include <string>

namespace aiedit::internal {

class SequenceChunk
{

  public:

    SequenceChunk(const std::string& seq,
                  size_t start_pos,
                  size_t end_pos,
                  unsigned num_hashes,
                  unsigned kmer_size);

    bool roll();
    const uint64_t* hashes() const;
    size_t get_pos() const;

  private:

    const std::string& seq;
    btllib::BlindNtHash hash_fn;
    const size_t end_pos;
};

}