#pragma once

#include <cstddef>
#include <memory>

namespace aiedit {

class Buffer2D
{

  public:

    Buffer2D(size_t num_rows, size_t num_cols);

    void set(size_t row, size_t col, float value);

    float get(size_t row, size_t col) const;

    float* data() const;

    size_t get_num_rows() const;

    size_t get_num_cols() const;

  private:

    size_t num_rows, num_cols;
    std::unique_ptr<float[]> data_ptr;
};

}