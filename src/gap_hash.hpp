#pragma once

#include <btllib/nthash.hpp>

namespace aiedit {

class GapHash
{
public:

  GapHash(const std::string& seq,
          unsigned num_hashes,
          unsigned kmer_size,
          unsigned max_gap,
          size_t pos = 0)
    : num_hashes(num_hashes)
  {
    hashers.emplace_back(seq, 1, kmer_size / 2, pos);
    for (unsigned gap = 1; gap <= max_gap; gap++) {
      const auto k = kmer_size - kmer_size / 2;
      hashers.emplace_back(seq, 1, k, k + pos + gap);
    }
  }

  bool roll()
  {
    bool can_roll = false;
    for (auto& hasher : hashers) {
      can_roll = hasher.roll();
    }
    return can_roll;
  }

  std::vector<uint64_t*> hashes()
  {
    std::vector<uint64_t*> del_hashes;
    const auto k = hashers[0].get_k() + hashers[1].get_k();
    const auto f_0 = hashers[0].get_forward_hash();
    const auto r_0 = hashers[0].get_reverse_hash();
    for (size_t i = 1; i < hashers.size(); i++) {
      auto f_i =
        btllib::hashing_internals::srol(f_0, hashers[i].get_k()) ^ hashers[i].get_forward_hash();
      auto r_i =
        r_0 ^ btllib::hashing_internals::srol(hashers[i].get_reverse_hash(), hashers[0].get_k());
      del_hashes.emplace_back(new uint64_t[num_hashes]);
      btllib::hashing_internals::extend_hashes(f_i, r_i, k, num_hashes, del_hashes[i - 1]);
    }
    return del_hashes;
  }

  size_t get_pos() { return hashers[0].get_pos(); }

private:

  const unsigned num_hashes;
  std::vector<btllib::NtHash> hashers;
};

};  // namespace aiedit