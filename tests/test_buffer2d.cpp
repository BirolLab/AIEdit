#include <ATen/ops/from_blob.h>
#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "core/buffer2d.hpp"

TEST_CASE("Test setter and getter", "[buffer2d]")
{
    aiedit::Buffer2D buffer(5, 3);
    REQUIRE(buffer.get_num_rows() == 5);
    REQUIRE(buffer.get_num_cols() == 3);
    buffer.set(0, 2, 1.0);
    REQUIRE(buffer.get(0, 2) == 1.0);
    buffer.set(1, 1, 0.5);
    REQUIRE(buffer.get(1, 1) == 0.5);
    REQUIRE(buffer.get(2, 0) == 0.0);
}

TEST_CASE("Test tensor conversion", "[buffer2d]")
{
    std::vector<std::vector<float>> data = {
      {0.2, 0.1, 0.3},
      {0.5, 0.2, 0.8}
    };
    const int num_rows = data.size(), num_cols = data[0].size();
    aiedit::Buffer2D buffer(num_rows, num_cols);
    for (size_t i = 0; i < num_rows; i++) {
        for (size_t j = 0; j < num_cols; j++) {
            buffer.set(i, j, data[i][j]);
        }
    }
    const auto tensor = at::from_blob(buffer.data(), {num_rows, num_cols});
    for (size_t i = 0; i < num_rows; i++) {
        for (size_t j = 0; j < num_cols; j++) {
            REQUIRE(tensor[i][j].item<float>() == data[i][j]);
        }
    }
}