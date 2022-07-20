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
  const unsigned frame_size, window_size;
  Signature signature;
  std::vector<btllib::SeedNtHash*> hash_fns;

  void update_signature();

public:
  Observer(const std::string& seq,
           const btllib::SeedBloomFilter& filter,
           const unsigned frame_size,
           const unsigned window_size)
    : filter(filter)
    , frame_size(frame_size)
    , window_size(window_size)
    , signature(frame_size, filter.get_seeds().size())
  {
    for (const auto& seed : filter.get_seeds()) {
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
  [[nodiscard]] size_t get_position() const;
};

}

#endif // AIEDIT_OBSERVER_HPP
