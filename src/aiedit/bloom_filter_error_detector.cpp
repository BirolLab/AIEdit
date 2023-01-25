#include "aiedit/error_detection/bloom_filter_error_detector.hpp"

namespace aiedit {

bool
BloomFilterErrorDetector::next_error()
{
    bool has_miss = false;
    while (seq_iter.has_next() && !has_miss) {
        seq_iter.next();
        has_miss = check_miss();
    }
    return has_miss;
}

bool
BloomFilterErrorDetector::check_miss()
{
    for (const uint64_t* seed_hashes : seq_iter.get_hashes()) {
        if (!bf.contains(seed_hashes)) {
            return true;
        }
    }
    return false;
}

}