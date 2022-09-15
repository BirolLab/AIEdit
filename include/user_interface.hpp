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
#include <filesystem>
#include <progressbar.hpp>
#include <string>

#include "pattern_database.hpp"

namespace ai_edit {

struct ProgramArguments
{
  std::filesystem::path assembly_path;
  std::filesystem::path bloom_filter_path;
  std::filesystem::path out_path;
  unsigned verbosity;
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
 * Class for printing a progress bar to stdout.
 */
class ProgressBar
{
private:
  progressbar* pbar;
  size_t file_size;
  bool show;
  unsigned percentage_done;
  size_t seq_info_bytes;

public:
  ProgressBar(size_t file_size, bool show)
    : file_size(file_size)
    , show(show)
    , percentage_done(0)
    , seq_info_bytes(0)
  {
    pbar = new progressbar(100, show);
    pbar->set_todo_char(" ");
    pbar->set_done_char("\033[1;34m█\033[0m");
    pbar->set_opening_bracket_char("");
    pbar->set_closing_bracket_char("");
  }

  /**
   * Add the bytes of a sequence's information to the progress.
   *
   * @param id Sequence ID.
   * @param comment Sequence comment.
   */
  void start_seq(const std::string& id, const std::string& comment);

  /**
   * Move the progress to a certain position in the sequence.
   *
   * @param miss_position The position of the current base in the sequence.
   */
  void seek(const size_t position);

  /**
   * Move the progress bar to the end.
   */
  void complete();

  bool is_shown() { return show; }
};

struct EditingLog
{
  unsigned num_patterns = 0;
  unsigned num_edits = 0;
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

/**
 * Print parsed command-line arguments to stdout.
 * @param args Object containing program arguments.
 */
void
print_args(const ai_edit::ProgramArguments& args);

/**
 * Print information about the pattern database to stdout.
 * @param database Popluated pattern database.
 */
void
print_database_information(const PatternDatabase& database,
                           const unsigned signature_length,
                           const unsigned num_seeds);

/**
 * Print the generated files' descriptions to stdout.
 */
void
print_output_files_list();

/**
 * Print the editing logs to stdout.
 */
void
print_editing_log(const EditingLog& log);

}

#endif // AI_EDIT_USER_INTERFACE_HPP