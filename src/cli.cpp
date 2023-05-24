#include "cli.hpp"
#include "version.hpp"

#define VERBOSITY_CHECK \
    if (!verbose) {     \
        return;         \
    }

namespace aiedit {

void CommandLineInterface::print_logo() const
{
    std::cout << LOGO << "\tv" << aiedit::VERSION << std::endl << std::endl;
}

void CommandLineInterface::print_args(const ProgramArguments& args) const
{
    VERBOSITY_CHECK
    std::cout << "- Assembly file         (-a)  = " << args.assembly_path << std::endl;
    std::cout << "- Bloom filter file     (-b)  = " << args.bf_path << std::endl;
    std::cout << "- Pattern detector file (-m)  = " << args.model_path << std::endl;
    std::cout << "- Output path           (-o)  = " << args.out_path << std::endl;
    std::cout << "- Number of threads     (-t)  = " << args.num_threads << std::endl;
    std::cout << std::endl;
}

void CommandLineInterface::print_bloom_filter_information(const btllib::SeedBloomFilter& bf) const
{
    VERBOSITY_CHECK
    std::cout << "- Size (bytes)       = " << bf.get_bytes() << std::endl;
    std::cout << "- FPR                = " << bf.get_fpr() << std::endl;
    std::cout << "- Occupancy          = " << bf.get_occupancy() << std::endl;
    std::cout << "- Hashes per seed    = " << bf.get_hash_num_per_seed() << std::endl;
    std::cout << "- Spaced seed length = " << bf.get_k() << std::endl;
    for (size_t i = 0; i < bf.get_seeds().size(); i++) {
        std::cout << "- Seed " << i + 1 << ": " << bf.get_seeds()[i] << std::endl;
    }
    std::cout << std::endl;
}

void CommandLineInterface::print_model_information(const nlohmann::json& model_json) const
{
    VERBOSITY_CHECK
    const auto keras_version = model_json["architecture"]["keras_version"];
    const auto backend = model_json["architecture"]["backend"];
    std::cout << "- Pattern length = " << model_json["pattern_length"] << std::endl;
    std::cout << "- Model hash     = " << model_json["hash"] << std::endl;
    std::cout << "- Keras version  = " << keras_version << std::endl;
    std::cout << "- Keras backend  = " << backend << std::endl;
    std::cout << std::endl;
}

void CommandLineInterface::print_polisher_results(const std::string& seq_id,
                                                  const PolishingResults& stats)
{
    VERBOSITY_CHECK
    const unsigned num_patterns = stats.get_num_ignored_patterns() + stats.get_num_fixed_patterns();
    std::cout << std::endl;
    std::cout << "[" << seq_id << "] ";
    std::cout << "fixed " << stats.get_num_fixed_patterns();
    std::cout << "/" << num_patterns << " patterns: ";
    std::cout << "M=" << stats.get_num_mismatches() << " ";
    std::cout << "I=" << stats.get_num_insertions() << " ";
    std::cout << "D=" << stats.get_num_deletions() << std::endl;
    std::cout << add_color("IGNORED:", Color::FG_RED);
    for (const auto& pos : stats.get_ignored_positions()) {
        std::cout << " " << pos;
    }
    std::cout << std::endl;
}
 
void CommandLineInterface::print_final_stats(const unsigned num_mismatches,
                                             const unsigned num_insertions,
                                             const unsigned num_deletions)
{
    std::cout << "Number of mismatches = " << num_mismatches << std::endl;
    std::cout << "Number of insertions = " << num_insertions << std::endl;
    std::cout << "Number of deletions  = " << num_deletions << std::endl;
}

void CommandLineInterface::start_timer(const std::string& message)
{
    std::cout << message << "... " << std::flush;
    timer.start();
}

void CommandLineInterface::stop_timer()
{
    timer.stop();
    std::cout << add_color("DONE", Color::FG_GREEN);
    std::cout << " (" << timer.to_string() << ")" << std::endl;
}

std::string CommandLineInterface::add_color(const std::string& text, Color color)
{
    return "\033[1;" + std::to_string(color) + "m" + text + "\033[0m";
}

}  // namespace aiedit