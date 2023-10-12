#ifndef CLI_HPP
#define CLI_HPP

#define LOGO                                         \
    "           _____ ______    _ _ _            \n" \
    "     /\\   |_   _|  ____|  | /_\\ |         \n" \
    "    /  \\    | | | |__   __| | | |_         \n" \
    "   / /\\ \\   | | |  __| / _` | | __|       \n" \
    "  / ____ \\ _| |_| |___| (_| | | |_         \n" \
    " /_/    \\_\\_____|______\\__,_|_|\\__|"

#include <btllib/counting_bloom_filter.hpp>
#include <nlohmann/json.hpp>

#include "args.hpp"
#include "polisher.hpp"
#include "timer.hpp"

namespace aiedit {

class CommandLineInterface
{

  public:

    CommandLineInterface(bool verbose)
      : verbose(verbose)
    {}

    /**
     * Print AIEdit's logo in ASCII art to stdout.
     */
    void print_logo() const;

    /**
     * Print parsed command-line arguments to stdout.
     * @param args Object containing program arguments.
     */
    void print_args(const ProgramArguments& args) const;

    /**
     * Print information about a btllib::SeedBloomFilter
     * @param bf The input Bloom filter.
     */
    void print_bloom_filter_info(const btllib::CountingBloomFilter8& bf) const;

    /**
     * Print information about a btllib::SeedBloomFilter
     * @param bf The input Bloom filter.
     */
    void print_model_info(const nlohmann::json& model_json) const;

    /**
     * Print verbose polishing statistics to stdout.
     */
    void print_polisher_results(const std::string& seq_id,
                                size_t seq_length,
                                unsigned thread_id,
                                const PolishingResults& stats);

    /**
     * Print final polishing statistics to stdout.
     */
    static void print_final_stats(const unsigned num_mismatches,
                                  const unsigned num_insertions,
                                  const unsigned num_deletions,
                                  const unsigned num_fixed_patterns,
                                  const unsigned num_ignored);

    /**
     * Start the timer
     * @param message Message to show while timing
     */
    void start_timer(const std::string& message);

    /**
     * Stop the timer
     */
    void stop_timer();

  private:

    const bool verbose;
    Timer timer;
};

}  // namespace aiedit

#endif  // CLI_HPP
