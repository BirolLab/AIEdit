#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include <nthash/nthash.hpp>

#include "aiedit/error_detection/bloom_filter_error_detector.hpp"
#include "tests/data.hpp"

TEST_CASE("Test Bloom filter error detector", "[error_detection]")
{
    MismatchTestData test_data;
    aiedit::SequenceIterator seq_iter(test_data.seq,
                                      test_data.seeds,
                                      test_data.num_hashes_per_seed);
    aiedit::BloomFilterErrorDetector bf_err_detector(seq_iter, *test_data.bf);
    REQUIRE(bf_err_detector.next_error());
    REQUIRE(seq_iter.get_position() == test_data.miss_position);
}