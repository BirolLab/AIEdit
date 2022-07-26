#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <queue>

#include "pattern_database.hpp"

TEST_CASE("Test database querying", "[database]")
{
  const std::string seed = "1101001011";
  const unsigned w = 5, n = seed.size();
  auto database = ai_edit::build_database({ seed }, w, n);
  auto signature = ai_edit::create_signature(n, 1);
  for (size_t i = 0; i < seed.size(); i++) {
    if (seed[i] == '1') {
      signature[i][0] = ai_edit::SignatureValue::MISS;
    } else {
      signature[i][0] = ai_edit::SignatureValue::HIT;
    }
  }
  auto query_result = ai_edit::query(signature, n, 1, database);
  REQUIRE(query_result.distance == 0);
  for (size_t i = 0; i < w; i++) {
    ai_edit::PatternValue expected;
    if (i == 0) {
      expected = ai_edit::PatternValue::MISMATCH;
    } else {
      expected = ai_edit::PatternValue::CLEAN;
    }
    REQUIRE(query_result.entry.pattern[i] == expected);
  }
}