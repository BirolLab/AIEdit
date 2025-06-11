#include "seed_generator.hpp"

#include <math.h>
#include <queue>
#include <set>

namespace {

std::string generate_seed(unsigned kmer_size, unsigned i_seed, std::mt19937& rng)
{
    std::string half = "1";
    kmer_size -= 2;
    std::uniform_real_distribution dist(0.0, 1.0);
    for (unsigned i = 0; i < (kmer_size + 1) / 2; i++) {
        const auto prob_lim = std::exp(-8.0 * (float)i / (float)kmer_size);
        half += dist(rng) < prob_lim ? '0' : '1';
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

float calculate_score(const std::vector<std::string>& seeds,
                      unsigned max_mismatches,
                      unsigned max_indels)
{
    float score = 0;
    if (std::set<std::string>(seeds.begin(), seeds.end()).size() != seeds.size()) {
        return score;
    }
    for (const auto& seed : seeds) {
        if (seed == std::string(seeds[0].size(), '1')) {
            return score;
        }
    }
    std::vector<std::string> patterns;
    generate_binary_strings(max_mismatches - 1, "0", patterns);
    for (unsigned n = 1; n <= max_indels; n++) {
        patterns.emplace_back(n, '0');
    }
    for (const auto& pattern : patterns) {
        unsigned containing_seeds = 0;
        for (const auto& seed : seeds) {
            if (seed.find(pattern) != std::string::npos) {
                ++containing_seeds;
            }
        }
        if (containing_seeds == 1) {
            ++score;
        }
    }
    score = score * (float)(max_mismatches + max_indels) / (float)patterns.size();
    float total_weight = 0;
    for (const auto& seed : seeds) {
        for (unsigned i = 0; i < seed.size(); i++) {
            if (seed[i] == '1') {
                ++total_weight;
            }
        }
    }
    score += total_weight / (float)seeds.size();
    return score;
}

std::pair<std::vector<std::string>, std::vector<std::string>> crossover(
  const std::vector<std::string>& seeds1, const std::vector<std::string>& seeds2, std::mt19937& rng)
{
    std::pair<std::vector<std::string>, std::vector<std::string>> result;
    std::bernoulli_distribution dist(0.5);
    for (unsigned i = 0; i < seeds1.size(); i++) {
        if (dist(rng)) {
            result.first.emplace_back(seeds1[i]);
            result.second.emplace_back(seeds2[i]);
        } else {
            result.first.emplace_back(seeds2[i]);
            result.second.emplace_back(seeds1[i]);
        }
    }
    return result;
}

void mutate(std::vector<std::string>& seeds, float probability, std::mt19937& rng)
{
    std::uniform_int_distribution<> dist(0, 1);
    for (auto& seed : seeds) {
        for (unsigned i = 1; i < seed.size() / 2; i++) {
            if (dist(rng) < probability) {
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
                             float mutation_probability,
                             std::optional<unsigned> random_seed)
  : population_size(population_size)
  , max_generations(max_generations)
  , mutation_probability(mutation_probability)
  , rng(random_seed.has_value() ? std::mt19937(random_seed.value())
                                : std::mt19937(std::random_device{}()))
{}

std::vector<std::string> SeedGenerator::generate(unsigned num_seeds,
                                                 unsigned kmer_size,
                                                 unsigned max_mismatches,
                                                 unsigned max_indels)
{
    std::priority_queue<std::pair<float, std::vector<std::string>>> population;
    for (unsigned i = 0; i < population_size; i++) {
        std::vector<std::string> seeds;
        for (unsigned j = 0; j < num_seeds; j++) {
            seeds.emplace_back(generate_seed(kmer_size, j, rng));
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
            auto new_seeds = crossover(seeds1, seeds2, rng);
            mutate(new_seeds.first, mutation_probability, rng);
            mutate(new_seeds.second, mutation_probability, rng);
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