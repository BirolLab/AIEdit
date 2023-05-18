#include "error_detector.hpp"

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
    for (const auto& seed_hashes : seq_iter.get_hashes()) {
        if (!bf.contains(seed_hashes)) {
            return true;
        }
    }
    return false;
}

}  // namespace aiedit