#ifndef AI_EDIT_USER_INTERFACE_HPP
#define AI_EDIT_USER_INTERFACE_HPP

#define PROGRAM "AIEdit"
#define VERSION "0.0.1"
#define DESCRIPTION "Artificially-intelligent long read genome polisher"
#define LOGO                                                                   \
  "           _____ ______    _ _ _            \n"                             \
  "     /\\   |_   _|  ____|  | /_\\ |         \n"                             \
  "    /  \\    | | | |__   __| | | |_         \n"                             \
  "   / /\\ \\   | | |  __| / _` | | __|       \n"                             \
  "  / ____ \\ _| |_| |___| (_| | | |_         \n"                             \
  " /_/    \\_\\_____|______\\__,_|_|\\__|"

#include <btllib/bloom_filter.hpp>
#include <chrono>
#include <string>

namespace ai_edit {

struct ProgramArguments
{
  std::string assembly_path;
  std::string bloom_filter_path;
  std::string out_path;
  bool verbose;
  unsigned signature_length;
  unsigned pattern_length;
  bool seq_reader_long_mode;
};

class Timer
{
private:
  std::clock_t t_start;
  std::clock_t t_end;

public:
  /**
   * Register the current time as the timer's starting point.
   */
  void start();

  /**
   * Register the current time as the timer's finish point.
   */
  void stop();

  /**
   * Compute the difference between the start and stop points in seconds.
   */
  [[nodiscard]] long double elapsed_seconds() const;

  /**
   * Get a human-readable representation of the elapsed time.
   */
  [[nodiscard]] std::string to_string() const;

  /**
   * Print a line-ender for logging.
   */
  void print_done() const;
};

/**
 * Read program parameters from the command line.
 * @param argc Number of command-line parameters given to main.
 * @param argv Value of the parameters passed to main.
 * @return ProgramArguments object containing the parsed parameters.
 */
ProgramArguments
parse_args(int argc, char** argv);

/**
 * Print AIEdit's logo in ASCII art to stdout.F
 */
void
print_logo();

/**
 * Print information about a btllib::SeedBloomFilter
 * @param bloom_filter The input Bloom filter.
 * @param path The path where the Bloom filter was read.
 */
void
print_bloom_filter_information(const btllib::SeedBloomFilter& filter,
                               const std::string& path);

}

#endif // AI_EDIT_USER_INTERFACE_HPP