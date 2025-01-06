#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "core/utils.hpp"

TEST_CASE("Test applying substitutions", "[apply_edits]")
{
    std::string before = "ACAGCAGTAGATG";
    std::string edited = "CCACCAGTAGATG";
    std::vector<aiedit::Edit> edits = {aiedit::Edit::substitution(0, 'A', 'C'),
                                       aiedit::Edit::substitution(3, 'G', 'C')};
    REQUIRE(aiedit::apply_edits(before, edits) == edited);
}

TEST_CASE("Test applying insertions", "[apply_edits]")
{
    std::string before = "ACAGCAGTAGATG";
    std::string edited = "ACATGGCAGTAGATG";
    std::vector<aiedit::Edit> edits = {aiedit::Edit::insertion(3, 'T'),
                                       aiedit::Edit::insertion(4, 'G')};
    REQUIRE(aiedit::apply_edits(before, edits) == edited);
}

TEST_CASE("Test applying deletions", "[apply_edits]")
{
    std::string before = "ACAGCAGTAGATG";
    std::string edited = "AGCAGTAGATG";
    std::vector<aiedit::Edit> edits = {aiedit::Edit::deletion(1, 'C'),
                                       aiedit::Edit::deletion(2, 'A')};
    REQUIRE(aiedit::apply_edits(before, edits) == edited);
}

TEST_CASE("Test applying multiple edits", "[apply_edits]")
{
    std::string before = "ACAGCAGTAGATG";
    std::string edited = "ATCGAGTAGATG";
    std::vector<aiedit::Edit> edits = {aiedit::Edit::deletion(1, 'C'),
                                       aiedit::Edit::deletion(2, 'A'),
                                       aiedit::Edit::substitution(3, 'G', 'T'),
                                       aiedit::Edit::insertion(5, 'G')};
    REQUIRE(aiedit::apply_edits(before, edits) == edited);
}