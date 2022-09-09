#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include <nthash/nthash.hpp>

#include "error_detection.hpp"
#include "tests/data.hpp"

TEST_CASE("Test miss position correctness", "[error_detection]")
{
  MismatchTestData test_data;
  nthash::SeedNtHash nthash(test_data.seq,
                            test_data.seeds,
                            test_data.num_hashes_per_seed,
                            test_data.seed_length);
  ai_edit::roll_to_next_miss(nthash, *test_data.bf);
  size_t miss_position = nthash.get_pos() + nthash.get_k() - 1;
  REQUIRE(miss_position == test_data.miss_position);
}