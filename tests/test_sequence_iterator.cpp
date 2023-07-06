#define CATCH_CONFIG_MAIN
#include <btllib/random_seq_generator.hpp>
#include <catch2/catch_test_macros.hpp>

#include "sequence_iterator.hpp"

TEST_CASE("substitution", "[sequence_iterator]")
{
    const std::string seq1 = "ACTGATCGACTGAGCT";
    const std::string seq2 = "ACTGACCGACTGAGCT";
    const unsigned position = 5;

    aiedit::SequenceIterator seq_iter1(seq1, {"11011"}, 3);
    aiedit::SequenceIterator seq_iter2(seq2, {"11011"}, 3);
    seq_iter1.next();
    seq_iter2.next();
    REQUIRE(seq_iter1.get_position() == position);
    REQUIRE(seq_iter2.get_position() == position);
    REQUIRE(seq_iter1.get_current() != seq_iter2.get_current());

    seq_iter2.substitute_last(seq1[position]);
    REQUIRE(seq_iter1.get_hashes(0)[0] == seq_iter2.get_hashes(0)[0]);
    REQUIRE(seq_iter1.get_current() == seq_iter2.get_current());
}