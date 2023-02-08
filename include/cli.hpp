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
    CommandLineInterface(unsigned verbosity)
      : verbosity(verbosity)
    {}

    /**
     * Log an edit to stdout
     * @param seq_id Sequence ID
     * @param fixed Indicates if a fix was detected
     * @param pattern_string String representation of the edit pattern
     * @param position Position of the pattern in the sequence
     * @param seq_len Sequence length
     */
    void log_edit(const std::string& seq_id,
                  bool fixed,
                  size_t position,
                  size_t seq_len);

    /**
     * Print AIEdit's logo in ASCII art to stdout.F
     */
    void print_logo();

    /**
     * Print information about a btllib::SeedBloomFilter
     * @param bf The input Bloom filter.
     */
    void print_bloom_filter_information(const btllib::SeedBloomFilter& bf);

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
    const unsigned verbosity;
    Timer timer;

    /**
     * Add unicode color to a string
     * @param text Text to be printed
     * @param color Color code
     * @return Colored string
     */
    std::string add_color(const std::string& text, Color color);
};

#endif // CLI_HPP
