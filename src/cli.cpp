#include "cli.hpp"
#include "aiedit/version.hpp"

void
CommandLineInterface::log_edit(const std::string& seq_id,
                               bool fixed,
                               size_t position,
                               size_t seq_len)
{
    if (verbosity < 2) {
        return;
    }
    Color c = (fixed ? Color::FG_GREEN : Color::FG_RED);
    std::string fixed_text = (fixed ? "FIXED  " : "IGNORED");
    std::cout << "[" << seq_id << "] ";
    std::cout << add_color(fixed_text, c) << "  ";
    std::cout << "@" << position << "/" << seq_len << "bp";
    std::cout << std::endl;
}

void
CommandLineInterface::print_logo()
{
    std::cout << LOGO << "\tv" << aiedit::VERSION << std::endl << std::endl;
}

void
CommandLineInterface::print_bloom_filter_information(const btllib::SeedBloomFilter& bf)
{
    if (verbosity < 1) {
        return;
    }
    std::cout << "- Size (bytes)       = " << bf.get_bytes() << std::endl;
    std::cout << "- FPR                = " << bf.get_fpr() << std::endl;
    std::cout << "- Occupancy          = " << bf.get_occupancy() << std::endl;
    std::cout << "- Hashes per seed    = " << bf.get_hash_num_per_seed() << std::endl;
    std::cout << "- Spaced seed length = " << bf.get_k() << std::endl;
    for (size_t i = 0; i < bf.get_seeds().size(); i++) {
        std::cout << "- Seed " << i + 1 << ": " << bf.get_seeds()[i] << std::endl;
    }
}

void
CommandLineInterface::print_args(const ProgramArguments& args)
{
    if (verbosity < 1) {
        return;
    }
    std::cout << "- Assembly file     (-a)  = " << args.assembly_path << std::endl;
    std::cout << "- Bloom filter file (-b)  = " << args.bf_path << std::endl;
    std::cout << "- Number of threads (-t)  = " << args.num_threads << std::endl;
    std::cout << "- Pattern length    (-w)  = " << args.pattern_length << std::endl;
    std::cout << std::endl;
}

void
CommandLineInterface::print_num_edits(unsigned num_patterns, unsigned num_mismatches)
{
    std::cout << "Number of error patterns = " << num_patterns << std::endl;
    std::cout << "Number of mismatches     = " << num_mismatches << std::endl;
}

void
CommandLineInterface::start_timer(const std::string& message)
{
    std::cout << message << "... " << std::flush;
    timer.start();
}

void
CommandLineInterface::stop_timer()
{
    timer.stop();
    std::cout << add_color("DONE", Color::FG_GREEN);
    std::cout << " (" << timer.to_string() << ")" << std::endl;
}

std::string
CommandLineInterface::add_color(const std::string& text, Color color)
{
    return "\033[1;" + std::to_string(color) + "m" + text + "\033[0m";
}