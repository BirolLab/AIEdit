#include "args.hpp"
#include "aiedit/version.hpp"

#include <argparse/argparse.hpp>
#include <iostream>

void ProgramArguments::parse(int argc, char** argv)
{
    auto parser = argparse::ArgumentParser("AIEdit", aiedit::VERSION);
    parser.add_description("Artificially-intelligent long read genome polisher");

    parser.add_argument("--assembly", "-a").help("Path to assembly file").required();

    parser.add_argument("--bloom-filter", "-b")
      .help("Path to btllib SeedBloomFilter populated with reads and seeds")
      .required();

    parser.add_argument("--model", "-m").help("Path to pattern detector model").required();

    parser.add_argument("--out-path", "-o")
      .help("Path to output directory for storing results")
      .default_value(".");

    parser.add_argument("--num-threads", "-t")
      .help("Number of sequences to process in parallel")
      .default_value((unsigned)8)
      .scan<'u', unsigned>();

    parser.add_argument("-V")
      .action([&](const auto&) { ++verbosity; })
      .append()
      .help("Level of details printed to stdout")
      .default_value(false)
      .implicit_value(true)
      .nargs(0);

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    assembly_path = std::filesystem::path(parser.get("-a"));
    bf_path = std::filesystem::path(parser.get("-b"));
    model_path = std::filesystem::path(parser.get("-m"));
    out_path = std::filesystem::path(parser.get("-o"));
    num_threads = parser.get<unsigned>("-t");
}