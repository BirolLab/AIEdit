#ifndef AIEDIT_SIGNATURE_HPP
#define AIEDIT_SIGNATURE_HPP

#include <btllib/bloom_filter.hpp>
#include <cstddef>
#include <string>
#include <vector>

#include "aiedit/sequence_iterator.hpp"

namespace aiedit {

class Signature
{
  public:
    Signature(size_t length, unsigned num_seeds)
    {
        for (unsigned i = 0; i < length; i++) {
            is_miss.push_back(std::vector<bool>(num_seeds, false));
        }
    }

    Signature(const SequenceIterator::HashVector* hashes, const btllib::SeedBloomFilter& bf)
    {
        is_miss.reserve(bf.get_seeds()[0].size());
        for (unsigned i = 0; i < bf.get_seeds()[0].size(); i++) {
            std::vector<bool> row(bf.get_seeds().size());
            for (unsigned j = 0; j < bf.get_seeds().size(); j++) {
                row.push_back(!bf.contains(hashes[i][j]));
            }
            is_miss.push_back(row);
        }
    }

    /**
     * Set a value for an element in the signature
     * @param position Position in the signature
     * @param seed_index Index of the spaced seed
     * @param is_miss `true` if there's a BF miss in `position` for the specific
     * seed, `false` otherwise
     */
    void set(size_t position, unsigned seed_index, bool has_miss);

    /**
     * Get the value of an element in the signature
     * @param position Position in the signature
     * @param seed_index Index of the spaced seed
     * @return `true` if there's a BF miss in `position` for the specific
     * seed, `false` otherwise
     */
    bool has_miss(size_t position, unsigned seed_index);

    /**
     * Get the signature as a vector of strings
     * @return Vector of strings, 'X' indicates a miss for a position/seed
     */
    std::vector<std::string> to_string_vector();

    /**
     * @return Length of the signature
     */
    size_t get_length() { return is_miss.size(); }

    /**
     * @return Number of spaced seeds in the signature
     */
    unsigned get_num_seeds() { return is_miss[0].size(); }

  private:
    std::vector<std::vector<bool>> is_miss;
};

}

#endif // AIEDIT_SIGNATURE_HPP