#ifndef AIEDIT_OBSERVER_HPP
#define AIEDIT_OBSERVER_HPP

#include "AIEdit/data_types.hpp"
#include <btllib/bloom_filter.hpp>
#include <btllib/nthash.hpp>
#include <string>
#include <vector>

namespace ai_edit {

class Observer
{
private:
  const btllib::SeedBloomFilter& filter;
  const unsigned frame_size;
  Signature signature;
  std::vector<btllib::SeedNtHash*> hash_fns;
  size_t position;

  void update_signature();

public:
  Observer(const std::string& seq,
           const btllib::SeedBloomFilter& filter,
           const unsigned frame_size)
    : filter(filter)
    , frame_size(frame_size)
    , signature(frame_size, filter.get_seeds().size())
  {
    position = 0;
    for (const auto& seed : filter.get_seeds()) {
      position = std::max(position, seed.size());
      auto* nth = new btllib::SeedNtHash(
        seq, { seed }, filter.get_hash_num(), seed.size());
      hash_fns.push_back(nth);
    }
  }

  /**
   * Observe the next frame.
   * @return `true` if the observer can continue sliding, otherwise `false`
   */
  bool next();

  [[nodiscard]] const Signature& get_signature() { return signature; }
  [[nodiscard]] size_t get_position() const { return position; }
};

}

#endif // AIEDIT_OBSERVER_HPP
