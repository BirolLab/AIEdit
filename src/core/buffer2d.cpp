#include "buffer2d.hpp"

namespace aiedit {

Buffer2D::Buffer2D(size_t num_rows, size_t num_cols)
  : num_rows(num_rows)
  , num_cols(num_cols)
  , data_ptr(std::make_unique<float[]>(num_rows * num_cols))
{}

void Buffer2D::set(size_t row, size_t col, float value) { data_ptr[row * num_cols + col] = value; }

float Buffer2D::get(size_t row, size_t col) const { return data_ptr[row * num_cols + col]; }

float* Buffer2D::data() const { return data_ptr.get(); }

size_t Buffer2D::get_num_rows() const { return num_rows; }

size_t Buffer2D::get_num_cols() const { return num_cols; }

}
