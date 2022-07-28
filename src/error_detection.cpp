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
  hash_fn.roll_back();
  uint64_t** hashes = hash_fn.peek_window(signature_length);
  hash_fn.roll();
  uint64_t* seed_hashes = new uint64_t[num_hashes_per_seed];
  for (size_t i = 0; i < signature_length; i++) {
    for (unsigned j = 0; j < num_seeds; j++) {
      std::copy(hashes[i] + j * num_hashes_per_seed,
                hashes[i] + (j + 1) * num_hashes_per_seed,
                seed_hashes);
      if (!filter.contains(seed_hashes)) {
        signature[i][j] = SignatureValue::MISS;
        has_miss = true;
      } else {
        signature[i][j] = SignatureValue::HIT;
      }
    }
  }
  return has_miss;
}