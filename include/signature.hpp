#ifndef AIEDIT_SIGNATURE_HPP
#define AIEDIT_SIGNATURE_HPP

#include <cstddef>

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

private:
  bool** is_miss;
  size_t length;
  unsigned num_seeds;
};

};

#endif // AIEDIT_SIGNATURE_HPP