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
      : length(length)
      , num_seeds(num_seeds)
    {
        is_miss = new bool*[length];
        for (unsigned i = 0; i < length; i++) {
            is_miss[i] = new bool[num_seeds];
        }
    }

    Signature(const SequenceIterator::HashVector* hashes, const btllib::SeedBloomFilter& bf)
      : Signature(bf.get_seeds()[0].size(), bf.get_seeds().size())
    {
        for (unsigned i = 0; i < length; i++) {
            for (unsigned j = 0; j < num_seeds; j++) {
                is_miss[i][j] = !bf.contains(hashes[i][j]);
            }
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

  private:
    bool** is_miss;
    size_t length;
    unsigned num_seeds;
};

}

#endif // AIEDIT_SIGNATURE_HPP