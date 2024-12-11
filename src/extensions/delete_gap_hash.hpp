#pragma once

#include <btllib/nthash.hpp>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class DeleteGapHash
{
  public:

    DeleteGapHash(const std::string& seq,
                  unsigned num_hashes,
                  unsigned kmer_size,
                  unsigned max_gap,
                  size_t pos = 0);

    bool roll();
    std::vector<uint64_t*> hashes() const;
    size_t get_pos() const;

  private:

    const unsigned num_hashes;
    std::vector<btllib::NtHash> hashers;
};

