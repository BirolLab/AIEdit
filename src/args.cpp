#include "args.hpp"
#include "version.hpp"

#include <argparse/argparse.hpp>
#include <iostream>

namespace aiedit {

void ProgramArguments::parse(int argc, char** argv)
{
    argparse::ArgumentParser parser("AIEdit", aiedit::VERSION);
    parser.add_description("Artificially-intelligent long read genome polisher");

    parser.add_argument("input_file").help("path to input file");

    parser.add_argument("--bloom-filter", "-b")
      .help("path to ntHits counting Bloom filter file")
      .required();

    parser.add_argument("--model", "-m").help("path to pattern detector model").required();

    parser.add_argument("--out-path", "-o")
      .help("output directory for storing results")
      .default_value(std::string(1, '.'));

    parser.add_argument("--num-threads", "-t")
      .help("number of threads to run in parallel")
      .default_value((unsigned)1)
      .scan<'u', unsigned>();

    parser.add_argument("--contig-mode")
      .help("optimize multithreading for polishing contigs/reads")
      .default_value(false)
      .implicit_value(true);

    parser.add_argument("--verbose")
      .help("print more details to stdout and log ignored patterns to ignored.tsv")
      .default_value(false)
      .implicit_value(true);

    help_message = parser.help().str();
    
    parser.parse_args(argc, argv);

    in_path = std::filesystem::path(parser.get("input_file"));
    bf_path = std::filesystem::path(parser.get("-b"));
    model_path = std::filesystem::path(parser.get("-m"));
    out_path = std::filesystem::path(parser.get("-o"));
    num_threads = parser.get<unsigned>("-t");
    contig_mode = parser.get<bool>("--contig-mode");
    verbose = parser.get<bool>("--verbose");
}

const std::string ProgramArguments::get_help_message() const { return help_message; }

}  // namespace aiedit