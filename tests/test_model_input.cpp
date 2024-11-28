#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include <btllib/counting_bloom_filter.hpp>

#include "feature_extraction.hpp"

TEST_CASE("Test alternating bases in model input", "[feature_extraction]")
{
    const std::string seq = "CATACGGGTTACGTACGT";
    btllib::CountingBloomFilter8 cbf(1024, 3);
    std::vector<float> probs = {1};
    const auto x = aiedit::get_model_input(seq, 0, 11, {"1111111"}, 1, cbf, probs);
    const auto x_true = torch::tensor({1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1});
    REQUIRE(x[0].equal(x_true));
}

TEST_CASE("Test model input for mismatches", "[feature_extraction]")
{
    const std::string ref = "CCATCGATGCTCAGGGCATCATGGCCAGTAGGG";
    const std::vector<std::string> seeds = {"1110111"};
    btllib::CountingBloomFilter8 cbf(1024, 3);
    std::vector<std::string> seqs = {"CATCGAT", "CATCGATG", "TCGATGATCAGGGCATC", "GGGCATC"};
    for (const auto& seq : seqs) {
        btllib::SeedNtHash nh(seq, seeds, cbf.get_hash_num(), seeds[0].size());
        while (nh.roll()) {
            cbf.insert(nh.hashes());
        }
    }
    const std::vector<float> probs = {1, 0.1, 0.2, 0.3, 0.4};
    const auto x = aiedit::get_model_input(ref, 1, 14, seeds, 1, cbf, probs);
    const auto x_true =
      torch::tensor({0.3, 0.3, 1.0, 1.0, 1.0, 0.1, 1.0, 1.0, 1.0, 0.1, 0.1, 0.1, 0.2});
    REQUIRE(x[1].equal(x_true));
}

TEST_CASE("Test model input for deletions", "[feature_extraction]")
{
    const std::string ref = "CCATCCGATGCTCAGGGGGCATCATGGCCAGTAGGG";
    btllib::CountingBloomFilter8 cbf(1024, 3);
    std::vector<std::string> seqs = {"CATCGAT", "CATCGATG", "TCGATGCTCAGGGCATC", "GGGCATC"};
    for (const auto& seq : seqs) {
        btllib::NtHash nh(seq, cbf.get_hash_num(), 7);
        while (nh.roll()) {
            cbf.insert(nh.hashes());
        }
    }
    const std::vector<float> probs = {1, 0.1, 0.2, 0.3, 0.4};
    const auto x = aiedit::get_model_input(ref, 1, 16, {"1111111"}, 2, cbf, probs);
    const auto x2_true =
      torch::tensor({1.0, 1.0, 1.0, 0.3, 0.3, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0});
    const auto x3_true =
      torch::tensor({1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.1, 0.1});
    REQUIRE(x[2].equal(x2_true));
    REQUIRE(x[3].equal(x3_true));
}