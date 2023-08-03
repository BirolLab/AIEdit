#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include <btllib/counting_bloom_filter.hpp>
#include <nthash/nthash.hpp>
#include <string>
#include <vector>

#include "error_corrector.hpp"
#include "pattern.hpp"
#include "polisher.hpp"

void populate(const std::string& seq,
              const std::vector<std::string>& seeds,
              btllib::CountingBloomFilter8& bf)
{
    aiedit::SequenceIterator seq_iter(seq, seeds, bf.get_hash_num());
    while (seq_iter.next()) {
        for (unsigned i = 0; i < seq_iter.get_num_seeds(); i++) {
            bf.insert(seq_iter.get_hashes(i));
        }
    }
}

TEST_CASE("single mismatch", "[error_corrector]")
{
    const std::string ref = "CATCGCGGCAT";
    const std::string alt = "CATCGTGGCAT";
    const unsigned position = 5;
    const std::vector<std::string> seeds = {"11111", "11011"};

    btllib::CountingBloomFilter8 bf(128, 3);
    populate(ref, seeds, bf);
    aiedit::ErrorCorrector ec(bf);

    aiedit::Pattern pattern(3);
    pattern.set(0, aiedit::Edit::Type::MISMATCH);
    aiedit::SequenceIterator seq_iter(alt, seeds, bf.get_hash_num());
    seq_iter.next();
    const auto edits = ec.fix(seq_iter, pattern);
    REQUIRE(edits.size() == 1);
    REQUIRE(edits.front().get_position() == position);
    REQUIRE(edits.front().get_before() == alt[position]);
    REQUIRE(edits.front().get_after() == ref[position]);
    REQUIRE(edits.front().get_type() == aiedit::Edit::Type::MISMATCH);

    aiedit::PolishingResults results;
    results.add_edits(edits);
    results.sort_edits();
    std::string edited = results.apply(alt);
    REQUIRE(edited == ref);
}