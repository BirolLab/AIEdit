#include "cli.hpp"
#include "aiedit/version.hpp"

void
ProgressBar::start_seq(const std::string& id, const std::string& comment)
{
    bytes_read += id.size() + comment.size();
    seq_position = 0;
}

void
ProgressBar::seek(const size_t position)
{
    if (!show) {
        return;
    }
    bytes_read += position - seq_position;
    seq_position = position;
    unsigned p = (unsigned)((double)bytes_read / (double)file_size * 100.0);
    for (unsigned i = 0; i < p - percentage_done; i++) {
        pbar->update();
    }
    percentage_done = p;
}

void
ProgressBar::complete()
{
    if (!show) {
        return;
    }
    for (unsigned i = 0; i < 100 - percentage_done; i++) {
        pbar->update();
    }
}

void
CommandLineInterface::print_logo()
{
    std::cout << LOGO << "\tv" << aiedit::VERSION << std::endl << std::endl;
}

void
CommandLineInterface::print_bloom_filter_information(const btllib::SeedBloomFilter& bf,
                                                     const std::string& path)
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
    if (verbosity < 2) {
        return;
    }
    std::cout << "Assembly file     (-a)  = " << args.assembly_path << std::endl;
    std::cout << "Bloom filter file (-b)  = " << args.bf_path << std::endl;
    std::cout << "Pattern length    (-w)  = " << args.pattern_length << std::endl;
    std::cout << std::endl;
}

void
CommandLineInterface::print_num_edits(unsigned num_patterns, unsigned num_mismatches)
{
    std::cout << "- Number of error patterns = " << num_patterns << std::endl;
    std::cout << "- Number of mismatches     = " << num_mismatches << std::endl;
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
    std::cout << "\033[1;32mDONE\033[0m";
    std::cout << " (" << timer.to_string() << ")" << std::endl;
}