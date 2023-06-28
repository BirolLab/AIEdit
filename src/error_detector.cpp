#include "error_detector.hpp"

#include <algorithm>

namespace aiedit {

bool ErrorDetector::next()
{
    bool has_miss = false;
    while (seq_iter.next() && !has_miss) {
        has_miss = bf.contains(seq_iter.get_hashes(0)) == 0;
    }
    return has_miss;
}

}  // namespace aiedit