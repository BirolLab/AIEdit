#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include <btllib/bloom_filter.hpp>
#include <btllib/nthash.hpp>

#include "data_types.hpp"
#include "editing.hpp"
#include "tests/data.hpp"

TEST_CASE("Test edit detection", "[editing]")
{
  MismatchTestData test_data;
  nthash::SeedNtHash nthash(test_data.seq,
                            test_data.seeds,
                            test_data.num_hashes_per_seed,
                            test_data.seed_length);
  nthash.roll();
  nthash.roll();
  REQUIRE(!test_data.bf->contains(nthash.hashes()));

  auto edits = ai_edit::get_edits(test_data.seq,
                                  test_data.miss_position,
                                  test_data.get_pattern(),
                                  test_data.pattern_length,
                                  *test_data.bf,
                                  nthash,
                                  test_data.seed_length);

  auto true_edits = test_data.get_true_edits();
  REQUIRE(edits.size() == true_edits.size());
  for (size_t i = 0; i < edits.size(); i++) {
    REQUIRE(edits[i].position == true_edits[i].position);
    REQUIRE(edits[i].content == true_edits[i].content);
  }
}

TEST_CASE("Test applying edits", "[editing]")
{
  MismatchTestData test_data;
  nthash::SeedNtHash nthash(test_data.seq,
                            test_data.seeds,
                            test_data.num_hashes_per_seed,
                            test_data.seed_length);
  nthash.roll();
  nthash.roll();
  std::string seq = test_data.seq;
  ai_edit::apply_edits(seq, nthash, test_data.get_true_edits());
  REQUIRE(seq == test_data.reference);
}