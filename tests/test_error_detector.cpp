#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include "data.hpp"

#include "error_detector.hpp"

TEST_CASE("Test Bloom filter error detector", "[error_detection]")
{
    MismatchTestData test_data;
    aiedit::SequenceIterator seq_iter(test_data.seq,
                                      test_data.seeds,
                                      test_data.num_hashes_per_seed);
    aiedit::ErrorDetector err_detector(seq_iter, test_data.bf);
    REQUIRE(err_detector.find_next());
    REQUIRE(seq_iter.get_position() == test_data.miss_position);
}