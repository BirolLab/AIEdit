#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include <btllib/bloom_filter.hpp>
#include <btllib/nthash.hpp>

#include "aiedit/error_correction/bloom_filter_mismatch_corrector.hpp"
#include "tests/data.hpp"

TEST_CASE("Test Bloom filter mismatch correction", "[error_correction]")
{
    MismatchTestData test_data;
    aiedit::SequenceIterator seq_iter(test_data.seq,
                                      test_data.seeds,
                                      test_data.num_hashes_per_seed);
    seq_iter.next(test_data.miss_position - seq_iter.get_seed_length() + 2);
    std::cout << seq_iter.get_position() << std::endl;
    aiedit::BloomFilterMismatchCorrector err_corrector(test_data.pattern_length, *test_data.bf);
    REQUIRE(err_corrector.fix(seq_iter));
    auto true_edits = test_data.get_true_edits();
    REQUIRE(err_corrector.get_edits().size() == true_edits.size());
    for (size_t i = 0; i < err_corrector.get_edits().size(); i++) {
        REQUIRE(err_corrector.get_edits()[i].position == true_edits[i].position);
        REQUIRE(err_corrector.get_edits()[i].after == true_edits[i].after);
    }
    REQUIRE(seq_iter.get_sequence() == test_data.reference);
}
