#include "error_detection.hpp"

bool
ai_edit::roll_to_next_miss(nthash::SeedNtHash& hash_fn,
                           const btllib::SeedBloomFilter& filter)
{
  while (true) {
    if (!hash_fn.roll()) {
      return false;
    }
    if (!filter.contains(hash_fn.hashes())) {
      return true;
    }
  }
}

bool
ai_edit::update_signature(nthash::SeedNtHash& hash_fn,
                          const btllib::SeedBloomFilter& filter,
                          Signature& signature,
                          const size_t signature_length)
{
  bool has_miss = false;
  unsigned num_seeds = filter.get_seeds().size();
  unsigned num_hashes_per_seed = hash_fn.get_hash_num_per_seed();
  uint64_t* hashes = new uint64_t[num_hashes_per_seed];
  for (size_t i = 0; i < signature_length; i++) {
    bool rolled = i > 0 ? hash_fn.roll() : true;
    for (unsigned j = 0; j < num_seeds; j++) {
      std::copy(hash_fn.hashes() + j * num_hashes_per_seed,
                hash_fn.hashes() + (j + 1) * num_hashes_per_seed,
                hashes);
      if (rolled && !filter.contains(hashes)) {
        signature[i][j] = SignatureValue::MISS;
        has_miss = true;
      } else {
        signature[i][j] = SignatureValue::HIT;
      }
    }
  }
  return has_miss;
}