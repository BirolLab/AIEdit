#include "error_detector.hpp"

#include <algorithm>

namespace aiedit {

bool ErrorDetector::find_next()
{
    bool has_miss = false;
    while (seq_iter.has_next() && !has_miss) {
        seq_iter.next();
        has_miss = is_miss();
    }
    return has_miss;
}

bool ErrorDetector::is_miss()
{
    const auto& hashes = seq_iter.get_hashes();
    auto bf_contains = [&](const std::vector<std::uint64_t>& h) { return bf.contains(h); };
    return !std::all_of(hashes.begin(), hashes.end(), bf_contains);
}

}  // namespace aiedit