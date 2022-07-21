#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include <AIEdit/database.hpp>
#include <iostream>
#include <queue>

TEST_CASE("Test database querying", "[database]")
{
  const std::string seed = "1101001011";
  const unsigned w = 5, n = seed.size();
  auto db = ai_edit::PatternDatabase(w, n, { seed });
  auto signature = ai_edit::Signature(n, 1);
  for (size_t i = 0; i < seed.size(); i++) {
    if (seed[i] == '1') {
      signature.set(i, 0, ai_edit::Signature::SignatureValue::MISS);
    } else {
      signature.set(i, 0, ai_edit::Signature::SignatureValue::HIT);
    }
  }
  unsigned distance;
  auto result = db.query(signature, distance);
  REQUIRE(distance == 0);
  for (size_t i = 0; i < w; i++) {
    ai_edit::Pattern::PatternValue expected;
    if (i == 0) {
      expected = ai_edit::Pattern::PatternValue::MISMATCH;
    } else {
      expected = ai_edit::Pattern::PatternValue::CLEAN;
    }
    REQUIRE(result.get_pattern().get(i) == expected);
  }
}