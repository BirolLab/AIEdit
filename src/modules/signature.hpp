#pragma once

#include <cstddef>
#include <memory>

namespace aiedit {

class Signature
{

  public:

    Signature(size_t length, size_t num_seeds);

    void set(size_t position, size_t i_seed, float value);

    float get(size_t position, size_t i_seed) const;

    float* data() const;

    size_t get_length() const;

    size_t get_num_seeds() const;

  private:

    size_t length, num_seeds;
    std::unique_ptr<float[]> data_ptr;
};

}