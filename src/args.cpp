#include "args.hpp"
#include "version.hpp"

#include <argparse/argparse.hpp>
#include <iostream>

namespace aiedit {

void ProgramArguments::parse(int argc, char** argv)
{
    auto parser = argparse::ArgumentParser("AIEdit", aiedit::VERSION);
    parser.add_description("Artificially-intelligent long read genome polisher");

    parser.add_argument("--assembly", "-a").help("path to assembly file").required();

    parser.add_argument("--bloom-filter", "-b")
      .help("path to btllib SeedBloomFilter file")
      .required();

    parser.add_argument("--model", "-m").help("path to pattern detector model").required();

    parser.add_argument("--out-path", "-o")
      .help("path to output directory for storing results")
      .required();

    parser.add_argument("--num-threads", "-t")
      .help("number of threads to run in parallel")
      .default_value((unsigned)1)
      .scan<'u', unsigned>();

    parser.add_argument("--contig-mode")
      .help("optimize multithreading for polishing contigs/raw reads")
      .default_value(false)
      .implicit_value(true);

    parser.add_argument("--verbose")
      .help("print more details to stdout and log ignored patterns to ignored.tsv")
      .default_value(false)
      .implicit_value(true);

    parser.parse_args(argc, argv);

    assembly_path = std::filesystem::path(parser.get("-a"));
    bf_path = std::filesystem::path(parser.get("-b"));
    model_path = std::filesystem::path(parser.get("-m"));
    out_path = std::filesystem::path(parser.get("-o"));
    num_threads = parser.get<unsigned>("-t");
    contig_mode = parser.get<bool>("--contig-mode");
    verbose = parser.get<bool>("--verbose");
}

}  // namespace aiedit