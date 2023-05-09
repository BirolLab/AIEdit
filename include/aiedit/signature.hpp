#ifndef AIEDIT_SIGNATURE_HPP
#define AIEDIT_SIGNATURE_HPP

#include <btllib/bloom_filter.hpp>
#include <cstddef>
#include <string>
#include <torch/script.h>

#include "aiedit/sequence_iterator.hpp"

namespace aiedit {

using ModelInput = std::vector<torch::jit::IValue>;

class Signature
{
  public:
    Signature(int length, unsigned num_seeds) { values = torch::zeros({ 1, length, num_seeds }); }

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
    size_t get_length();

    /**
     * @return Number of spaced seeds in the signature
     */
    size_t get_num_seeds();

    /**
     * @return Tensor representing the signature
     */
    const torch::Tensor& get_tensor() const { return values; }

  private:
    torch::Tensor values;
};

}

#endif // AIEDIT_SIGNATURE_HPP