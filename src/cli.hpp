#ifndef CLI_HPP
#define CLI_HPP

#define LOGO                                         \
    "           _____ ______    _ _ _            \n" \
    "     /\\   |_   _|  ____|  | /_\\ |         \n" \
    "    /  \\    | | | |__   __| | | |_         \n" \
    "   / /\\ \\   | | |  __| / _` | | __|       \n" \
    "  / ____ \\ _| |_| |___| (_| | | |_         \n" \
    " /_/    \\_\\_____|______\\__,_|_|\\__|"

#include <btllib/bloom_filter.hpp>
#include <nlohmann/json.hpp>

#include "args.hpp"
#include "polisher.hpp"
#include "timer.hpp"

namespace aiedit {

class CommandLineInterface
{
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

  public:

    CommandLineInterface(bool verbose) : verbose(verbose) {}

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
    void print_bloom_filter_information(const btllib::SeedBloomFilter& bf) const;

    /**
     * Print information about a btllib::SeedBloomFilter
     * @param bf The input Bloom filter.
     */
    void print_model_information(const nlohmann::json& model_json) const;

    /**
     * Print verbose polishing statistics to stdout.
     */
    void print_polisher_results(const std::string& seq_id, const PolishingResults& stats);

    /**
     * Print final polishing statistics to stdout.
     */
    static void print_final_stats(const unsigned num_mismatches,
                                  const unsigned num_insertions,
                                  const unsigned num_deletions);

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

    /**
     * Add unicode color to a string
     * @param text Text to be printed
     * @param color Color code
     * @return Colored string
     */
    static std::string add_color(const std::string& text, Color color);
};

}  // namespace aiedit

#endif  // CLI_HPP
