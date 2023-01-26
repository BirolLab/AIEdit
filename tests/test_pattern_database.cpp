#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <queue>

#include "aiedit/pattern_detection/pattern_database.hpp"

TEST_CASE("Test pattern database querying", "[pattern_detection]")
{
    const std::vector<std::string> seeds = { "1101001011" };
    const unsigned w = 5;
    aiedit::PatternDatabase database(w, seeds);
    aiedit::Signature signature(seeds[0].size(), 1);
    for (size_t i = 0; i < seeds[0].size(); i++) {
        signature.set(i, 0, seeds[0][i] == '1');
    }
    auto query_result = database.get_pattern(signature);
    for (size_t i = 0; i < w; i++) {
        if (i == 0) {
            REQUIRE(query_result.get(i) == aiedit::Edit::Type::MISMATCH);
        } else {
            REQUIRE(query_result.get(i) == aiedit::Edit::Type::NONE);
        }
    }
}