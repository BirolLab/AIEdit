#pragma once

#include <map>
#include <optional>
#include <random>
#include <string>
#include <vector>

namespace aiedit {

class SeedGenerator
{

  public:

    SeedGenerator(unsigned population_size,
                  unsigned max_generations,
                  float mutation_probability,
                  std::optional<unsigned> random_seed = std::nullopt);

    std::vector<std::string>
    generate(unsigned num_seeds, unsigned kmer_size, unsigned max_mismatches, unsigned max_indels);

  private:

    const unsigned population_size;
    const unsigned max_generations;
    const float mutation_probability;
    std::mt19937 rng;
};

}