#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include "gap_hash.hpp"

TEST_CASE("Test GapHash deletion (even k)", "[gap_hash]")
{
    std::string seq = "GGCATGCGATGCTC";
    std::vector<std::string> del_kmers_g1 = {"ATGCATGC", "TGCGTGCT"};
    std::vector<std::string> del_kmers_g2 = {"ATGCTGCT", "TGCGGCTC"};
    unsigned kmer_size = del_kmers_g1[0].size();

    aiedit::GapHash gh(seq, 1, kmer_size, 2, 3);

    REQUIRE(gh.roll());
    btllib::NtHash nh1(del_kmers_g1[0], 1, kmer_size);
    nh1.roll();
    REQUIRE(gh.hashes()[0][0] == nh1.hashes()[0]);
    btllib::NtHash nh2(del_kmers_g2[0], 1, kmer_size);
    nh2.roll();
    REQUIRE(gh.hashes()[1][0] == nh2.hashes()[0]);

    REQUIRE(gh.roll());
    btllib::NtHash nh3(del_kmers_g1[1], 1, kmer_size);
    nh3.roll();
    REQUIRE(gh.hashes()[0][0] == nh3.hashes()[0]);
    btllib::NtHash nh4(del_kmers_g2[1], 1, kmer_size);
    nh4.roll();
    REQUIRE(gh.hashes()[1][0] == nh4.hashes()[0]);

    REQUIRE(!gh.roll());
}

TEST_CASE("Test GapHash deletion (odd k)", "[gap_hash]")
{
    std::string seq = "CTATGCGATGCTC";
    std::vector<std::string> del_kmers_g1 = {"ATGGATG", "TGCATGC"};
    std::vector<std::string> del_kmers_g2 = {"ATGATGC", "TGCTGCT"};
    unsigned kmer_size = del_kmers_g1[0].size();

    aiedit::GapHash gh(seq, 1, kmer_size, 2, 2);

    REQUIRE(gh.roll());
    btllib::NtHash nh1(del_kmers_g1[0], 1, kmer_size);
    nh1.roll();
    REQUIRE(gh.hashes()[0][0] == nh1.hashes()[0]);
    btllib::NtHash nh2(del_kmers_g2[0], 1, kmer_size);
    nh2.roll();
    REQUIRE(gh.hashes()[1][0] == nh2.hashes()[0]);

    REQUIRE(gh.roll());
    btllib::NtHash nh3(del_kmers_g1[1], 1, kmer_size);
    nh3.roll();
    REQUIRE(gh.hashes()[0][0] == nh3.hashes()[0]);
    btllib::NtHash nh4(del_kmers_g2[1], 1, kmer_size);
    nh4.roll();
    REQUIRE(gh.hashes()[1][0] == nh4.hashes()[0]);
}