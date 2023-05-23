#include "error_detector.hpp"

#include <algorithm>

namespace aiedit {

bool ErrorDetector::next()
{
    bool has_miss = false;
    while (seq_iter.has_next() && !has_miss) {
        seq_iter.next();
        has_miss = is_miss();
    }
    return has_miss;
}

bool ErrorDetector::is_miss() { return !bf.contains(seq_iter.get_hashes()[0]); }

}  // namespace aiedit