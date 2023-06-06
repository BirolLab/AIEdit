#include "error_detector.hpp"

#include <algorithm>

namespace {

using namespace aiedit;

/**
 * Check if the current position of the sequence iterator is a miss
 * @return `true` if the position is a miss
 */
inline bool is_miss(const SequenceIterator::HashVector& hashes,
                    const btllib::CountingBloomFilter8& bf)
{
    return bf.contains(hashes[0]) == 0;
}

}  // namespace
namespace aiedit {

bool ErrorDetector::next()
{
    bool has_miss = false;
    while (seq_iter.has_next() && !has_miss) {
        seq_iter.next();
        has_miss = is_miss(seq_iter.get_hashes(), bf);
    }
    return has_miss;
}

}  // namespace aiedit