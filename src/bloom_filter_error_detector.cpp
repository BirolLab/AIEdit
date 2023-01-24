#include "error_detection/bloom_filter_error_detector.hpp"

namespace aiedit {

bool
BloomFilterErrorDetector::has_error()
{
  return seq_iter.has_next();
}

void
BloomFilterErrorDetector::next_error()
{
  bool is_miss = false;
  while (seq_iter.has_next() && !is_miss) {
    seq_iter.next();
    is_miss = check_miss();
  }
}

bool
BloomFilterErrorDetector::check_miss()
{
  for (const auto& seed_hashes : seq_iter.get_hashes()) {
    if (!bf.contains(seed_hashes)) {
      return true;
    }
  }
  return false;
}

};