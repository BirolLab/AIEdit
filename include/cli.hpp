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
#include <progressbar.hpp>

#include "args.hpp"
#include "timer.hpp"

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
    size_t bytes_read;
    size_t seq_position;

  public:
    ProgressBar(size_t file_size, bool show)
      : file_size(file_size)
      , show(show)
      , percentage_done(0)
      , bytes_read(0)
      , seq_position(0)
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
    void seek(const uint64_t position);

    /**
     * Move the progress bar to the end.
     */
    void complete();

    bool is_shown() { return show; }
};

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
