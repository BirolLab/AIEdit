#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include "sequence_iterator.hpp"

TEST_CASE("chunks", "[sequence_iterator]")
{
    const std::string seq = "TACGTAGCATGATGCTAGC";
    const size_t begin = 3;
    const size_t end = 10;
    const std::string chunk = seq.substr(begin, end - begin);

    aiedit::SequenceIterator seq_iter1(seq, {"11011"}, 1, begin, end);
    aiedit::SequenceIterator seq_iter2(chunk, {"11011"}, 1);
    bool next1 = seq_iter1.next();
    bool next2 = seq_iter2.next();
    while (next1 && next2) {
        REQUIRE(seq_iter1.get_current() == seq_iter2.get_current());
        REQUIRE(seq_iter1.get_hashes(0)[0] == seq_iter2.get_hashes(0)[0]);
        next1 = seq_iter1.next();
        next2 = seq_iter2.next();
    }
    REQUIRE(next1 == next2);
}

TEST_CASE("substitution", "[sequence_iterator]")
{
    const std::string ref = "ACTGATCGACTGAGCT";
    const std::string alt = "ACTGACCGACTGAGCT";
    const unsigned position = 5;

    aiedit::SequenceIterator ref_iter(ref, {"11011"}, 1);
    aiedit::SequenceIterator alt_iter(alt, {"11011"}, 1);
    ref_iter.next();
    alt_iter.next();
    REQUIRE(ref_iter.get_position() == position);
    REQUIRE(alt_iter.get_position() == position);
    REQUIRE(ref_iter.get_current() != alt_iter.get_current());

    alt_iter.substitute_last(ref[position]);
    REQUIRE(ref_iter.get_current() == alt_iter.get_current());
    REQUIRE(ref_iter.get_position() == alt_iter.get_position());
    REQUIRE(ref_iter.get_hashes(0)[0] == alt_iter.get_hashes(0)[0]);
}

TEST_CASE("insertion", "[sequence_iterator]")
{
    const std::string ref = "ACTGACGTACGTAGCTAC";
    const std::string alt = "ACTGAGTACGTAGCTAC";
    const unsigned position = 5;

    aiedit::SequenceIterator ref_iter(ref, {"11011"}, 1);
    aiedit::SequenceIterator alt_iter(alt, {"11011"}, 1);
    ref_iter.next();
    alt_iter.next();
    REQUIRE(ref_iter.get_position() == position);
    REQUIRE(alt_iter.get_position() == position);
    REQUIRE(ref_iter.get_current() != alt_iter.get_current());

    alt_iter.insert_last(ref[position]);
    REQUIRE(ref_iter.get_current() == alt_iter.get_current());
    REQUIRE(ref_iter.get_position() == alt_iter.get_position());
    REQUIRE(ref_iter.get_hashes(0)[0] == alt_iter.get_hashes(0)[0]);
}

TEST_CASE("deletion", "[sequence_iterator]")
{
    const std::string ref = "ACTGAGTACGTAGCTAC";
    const std::string alt = "ACTGACGTACGTAGCTAC";
    const unsigned position = 5;

    aiedit::SequenceIterator ref_iter(ref, {"11011"}, 1);
    aiedit::SequenceIterator alt_iter(alt, {"11011"}, 1);
    ref_iter.next();
    alt_iter.next();
    REQUIRE(ref_iter.get_position() == position);
    REQUIRE(alt_iter.get_position() == position);
    REQUIRE(ref_iter.get_current() != alt_iter.get_current());

    alt_iter.delete_last();
    REQUIRE(ref_iter.get_current() == alt_iter.get_current());
    REQUIRE(ref_iter.get_position() == alt_iter.get_position() - 1);
    REQUIRE(ref_iter.get_hashes(0)[0] == alt_iter.get_hashes(0)[0]);
}