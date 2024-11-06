#pragma once

#define LOGO                                         \
    "           _____ ______    _ _ _            \n" \
    "     /\\   |_   _|  ____|  | /_\\ |         \n" \
    "    /  \\    | | | |__   __| | | |_         \n" \
    "   / /\\ \\   | | |  __| / _` | | __|       \n" \
    "  / ____ \\ _| |_| |___| (_| | | |_         \n" \
    " /_/    \\_\\_____|______\\__,_|_|\\__|"

#include <btllib/counting_bloom_filter.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

#include "args.hpp"
#include "polisher.hpp"
#include "timer.hpp"

#define VERBOSITY_CHECK \
    if (!verbose) {     \
        return;         \
    }

enum Color
{
    FG_RED = 31,
    FG_GREEN = 32,
    FG_BLUE = 34,
    FG_YELLOW = 33,
    FG_DEFAULT = 39,
    BG_RED = 41,
    BG_GREEN = 42,
    BG_BLUE = 44,
    BG_YELLOW = 43,
    BG_DEFAULT = 49
};

/**
 * Add unicode coloring to a string
 * @param text Text to be printed
 * @param color Color code
 * @return Colored string
 */
inline const std::string add_color(const std::string& text, Color color)
{
    return "\033[1;" + std::to_string(color) + "m" + text + "\033[0m";
}

class CommandLineInterface
{

  public:

    CommandLineInterface(bool verbose, const std::string& version)
      : verbose(verbose)
      , version(version)
    {}

    /**
     * Print AIEdit's logo in ASCII art to stdout.
     */
    void print_logo() const { std::cout << LOGO << "\tv" << version << std::endl << std::endl; }

    /**
     * Print parsed command-line arguments to stdout.
     * @param args Object containing program arguments.
     */
    void print_args(const ProgramArguments& args) const
    {
        VERBOSITY_CHECK
        std::cout << "- Input file                  = " << args.in_path << std::endl;
        std::cout << "- Bloom filter file     (-b)  = " << args.bf_path << std::endl;
        std::cout << "- Pattern detector file (-m)  = " << args.model_path << std::endl;
        std::cout << "- Output path           (-o)  = " << args.out_path << std::endl;
        std::cout << "- Number of threads     (-t)  = " << args.num_threads << std::endl;
        std::string mode = args.contig_mode ? "CONTIGS" : "DRAFT";
        std::cout << "- Multithreading mode         = " << mode << std::endl;
        std::cout << std::endl;
    }

    /**
     * Print information about a btllib::SeedBloomFilter
     * @param bf The input Bloom filter.
     */
    void print_bloom_filter_info(const btllib::CountingBloomFilter8& bf) const
    {
        VERBOSITY_CHECK
        std::cout << "- Size (bytes)       = " << bf.get_bytes() << std::endl;
        std::cout << "- FPR                = " << bf.get_fpr() << std::endl;
        std::cout << "- Occupancy          = " << bf.get_occupancy() << std::endl;
        std::cout << "- Hashes per seed    = " << bf.get_hash_num() << std::endl;
        std::cout << std::endl;
    }

    /**
     * Print information about a btllib::SeedBloomFilter
     * @param bf The input Bloom filter.
     */
    void print_model_info(const nlohmann::json& model_json) const
    {
        VERBOSITY_CHECK
        const auto keras_version = model_json["architecture"]["keras_version"];
        const auto backend = model_json["architecture"]["backend"];
        std::cout << "- Pattern length = " << model_json["pattern_length"] << std::endl;
        std::cout << "- Model hash     = " << model_json["hash"] << std::endl;
        std::cout << "- Keras version  = " << keras_version << std::endl;
        std::cout << "- Keras backend  = " << backend << std::endl;
        const std::vector<std::string> seeds = model_json["seeds"];
        std::cout << "- Spaced seed length = " << seeds[0].size() << std::endl;
        for (unsigned i = 0; i < seeds.size(); i++) {
            std::cout << "- Seed " << i + 1 << ": " << seeds[i] << std::endl;
        }
        std::cout << std::endl;
    }

    /**
     * Print verbose polishing statistics to stdout.
     */
    void print_polisher_results(const std::string& seq_id,
                                size_t seq_length,
                                unsigned thread_id,
                                const PolishingResults& stats)
    {
        VERBOSITY_CHECK
        const unsigned num_patterns =
          stats.get_num_ignored_patterns() + stats.get_num_fixed_patterns();
#pragma omp critical
        {
            std::cout << "[" << seq_id << ", thread " << thread_id << "] ";
            std::cout << "fixed " << stats.get_num_fixed_patterns();
            std::cout << "/" << num_patterns << " patterns in " << seq_length << " bps: ";
            std::cout << "M=" << stats.get_num_mismatches() << " ";
            std::cout << "I=" << stats.get_num_insertions() << " ";
            std::cout << "D=" << stats.get_num_deletions() << std::endl;
        }
    }

    /**
     * Print final polishing statistics to stdout.
     */
    static void print_final_stats(const unsigned num_mismatches,
                                  const unsigned num_insertions,
                                  const unsigned num_deletions,
                                  const unsigned num_fixed_patterns,
                                  const unsigned num_ignored)
    {
        std::cout << "Number of patterns fixed    = " << num_fixed_patterns << std::endl;
        std::cout << "Number of patterns ignored  = " << num_ignored << std::endl;
        std::cout << "Number of bases mutated     = " << num_mismatches << std::endl;
        std::cout << "Number of bases inserted    = " << num_insertions << std::endl;
        std::cout << "Number of bases deleted     = " << num_deletions << std::endl;
    }

    /**
     * Start the timer
     * @param message Message to show while timing
     */
    void start_timer(const std::string& message)
    {
        std::cout << message << "... " << std::flush;
        timer.start();
    }

    /**
     * Stop the timer
     */
    void stop_timer()
    {
        timer.stop();
        std::cout << add_color("DONE", Color::FG_GREEN);
        std::cout << " (" << timer.to_string() << ")" << std::endl;
    }

  private:

    const bool verbose;
    const std::string& version;
    Timer timer;
};
