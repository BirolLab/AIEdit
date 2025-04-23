#include "seed_generator.hpp"

#include <queue>
#include <random>

namespace {

std::string generate_seed(unsigned kmer_size)
{
    std::string half = "1";
    kmer_size -= 2;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(0.8);
    for (unsigned i = 0; i < (kmer_size + 1) / 2; i++) {
        half += dist(gen) ? '1' : '0';
    }
    std::string seed = half;
    if (kmer_size % 2 == 0) {
        seed += std::string(half.rbegin(), half.rend());
    } else {
        seed += std::string(half.rbegin() + 1, half.rend());
    }
    return seed;
}

void generate_binary_strings(unsigned k, std::string current, std::vector<std::string>& results)
{
    if (k == 0) {
        results.push_back(current);
        return;
    }
    generate_binary_strings(k - 1, current + "0", results);
    generate_binary_strings(k - 1, current + "1", results);
}

unsigned
calculate_score(const std::vector<std::string>& seeds, unsigned max_mismatches, unsigned max_indels)
{
    unsigned score = 0;
    std::vector<std::string> mismatch_patterns;
    generate_binary_strings(max_mismatches - 1, "0", mismatch_patterns);
    for (const auto& pattern : mismatch_patterns) {
        for (const auto& seed : seeds) {
            if (seed.find(pattern) != std::string::npos) {
                ++score;
                break;
            }
        }
    }
    unsigned total_weight = 0;
    for (const auto& seed : seeds) {
        for (unsigned i = 0; i < seed.size(); i++) {
            if (seed[i] == '1') {
                ++total_weight;
            }
        }
    }
    score += total_weight / (5 * seeds.size());
    return score;
}

std::pair<std::vector<std::string>, std::vector<std::string>>
crossover(const std::vector<std::string>& seeds1, const std::vector<std::string>& seeds2)
{
    std::pair<std::vector<std::string>, std::vector<std::string>> result;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(0.5);
    for (unsigned i = 0; i < seeds1.size(); i++) {
        if (dist(gen)) {
            result.first.emplace_back(seeds1[i]);
            result.second.emplace_back(seeds2[i]);
        } else {
            result.first.emplace_back(seeds2[i]);
            result.second.emplace_back(seeds1[i]);
        }
    }
    return result;
}

void mutate(std::vector<std::string>& seeds, float probability)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 1);
    for (auto& seed : seeds) {
        for (unsigned i = 1; i < seed.size() / 2; i++) {
            if (dist(gen) < probability) {
                seed[i] = seed[i] == '1' ? '0' : '1';
                seed[seed.size() - 1 - i] = seed[seed.size() - 1 - i] == '1' ? '0' : '1';
            }
        }
    }
}

}

namespace aiedit {

SeedGenerator::SeedGenerator(unsigned population_size,
                             unsigned max_generations,
                             float mutation_probability)
  : population_size(population_size)
  , max_generations(max_generations)
  , mutation_probability(mutation_probability)
{}

std::vector<std::string> SeedGenerator::generate(unsigned num_seeds,
                                                 unsigned kmer_size,
                                                 unsigned max_mismatches,
                                                 unsigned max_indels)
{
    std::priority_queue<std::pair<unsigned, std::vector<std::string>>> population;
    for (unsigned i = 0; i < population_size; i++) {
        std::vector<std::string> seeds;
        for (unsigned j = 0; j < num_seeds; j++) {
            seeds.emplace_back(generate_seed(kmer_size));
        }
        const auto score = calculate_score(seeds, max_mismatches, max_indels);
        population.emplace(std::make_pair(score, seeds));
    }
    for (unsigned i = 0; i < max_generations; i++) {
        for (unsigned j = 0; j < population.size(); j++) {
            std::priority_queue<std::pair<unsigned, std::vector<std::string>>> temp;
            const auto seeds1 = population.top().second;
            temp.emplace(population.top());
            population.pop();
            const auto seeds2 = population.top().second;
            temp.emplace(population.top());
            population.pop();
            auto new_seeds = crossover(seeds1, seeds2);
            mutate(new_seeds.first, mutation_probability);
            mutate(new_seeds.second, mutation_probability);
            const auto score1 = calculate_score(new_seeds.first, max_mismatches, max_indels);
            const auto score2 = calculate_score(new_seeds.second, max_mismatches, max_indels);
            temp.emplace(std::make_pair(score1, new_seeds.first));
            temp.emplace(std::make_pair(score2, new_seeds.second));
            population.emplace(temp.top());
            temp.pop();
            population.emplace(temp.top());
            temp.pop();
        }
    }
    return population.top().second;
}

}