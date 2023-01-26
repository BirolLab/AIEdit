#ifndef CLI_HPP
#define CLI_HPP

#define LOGO                                                                                       \
    "           _____ ______    _ _ _            \n"                                               \
    "     /\\   |_   _|  ____|  | /_\\ |         \n"                                               \
    "    /  \\    | | | |__   __| | | |_         \n"                                               \
    "   / /\\ \\   | | |  __| / _` | | __|       \n"                                               \
    "  / ____ \\ _| |_| |___| (_| | | |_         \n"                                               \
    " /_/    \\_\\_____|______\\__,_|_|\\__|"

#include <btllib/bloom_filter.hpp>

#include "args.hpp"
#include "timer.hpp"

class CommandLineInterface
{
  public:
    CommandLineInterface(unsigned verbosity)
      : verbosity(verbosity)
    {}

    /**
     * Print AIEdit's logo in ASCII art to stdout.F
     */
    void print_logo();

    /**
     * Print information about a btllib::SeedBloomFilter
     * @param bloom_filter The input Bloom filter.
     * @param path The path where the Bloom filter was read.
     */
    void print_bloom_filter_information(const btllib::SeedBloomFilter& filter,
                                        const std::string& path);

    /**
     * Print parsed command-line arguments to stdout.
     * @param args Object containing program arguments.
     */
    void print_args(const ProgramArguments& args);

    /**
     * Print the number of edits to stdout.
     */
    void print_num_edits(unsigned num_patterns, unsigned num_mismatches);

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
    unsigned verbosity;
    Timer timer;
};

#endif // CLI_HPP
