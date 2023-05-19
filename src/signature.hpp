#ifndef AIEDIT_SIGNATURE_HPP
#define AIEDIT_SIGNATURE_HPP

#include <btllib/bloom_filter.hpp>
#include <cstddef>
#include <fdeep/fdeep.hpp>
#include <string>

#include "sequence_iterator.hpp"

namespace aiedit {

class Signature
{
  public:

    Signature(unsigned length, unsigned num_seeds)
      : values(fdeep::tensor_shape(length, num_seeds), 0)
    {}

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
     * @return Length of the signature
     */
    size_t get_length();

    /**
     * @return Number of spaced seeds in the signature
     */
    size_t get_num_seeds();

    /**
     * @return Tensor representing the signature
     */
    const fdeep::tensor& get_tensor() const { return values; }

  private:

    fdeep::tensor values;
};

}  // namespace aiedit

#endif  // AIEDIT_SIGNATURE_HPP