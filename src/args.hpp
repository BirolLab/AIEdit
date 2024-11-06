#pragma once

#include <filesystem>
#include <string>
#include <argparse/argparse.hpp>
#include <iostream>

class ProgramArguments
{
  public:

    std::filesystem::path in_path;
    std::filesystem::path bf_path;
    std::filesystem::path model_path;
    std::filesystem::path out_path;
    unsigned num_threads;
    bool contig_mode;
    bool no_apply;
    bool verbose;

    /**
     * Parse command-line arguments
     * @param argc Number of input arguments given to the main(...) function
     * @param argv Argument data from the main(...) function
     * @return Arguments object containing parsed arguments
     */
    void parse(int argc, char** argv, const std::string& version)
    {
        argparse::ArgumentParser parser("AIEdit", version);
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

        parser.add_argument("--no-apply")
          .help("don't apply edits and skip writing to edited.fa")
          .default_value(false)
          .implicit_value(true);

        parser.add_argument("--verbose")
          .help("print more details to stdout and log ignored patterns to ignored.tsv")
          .default_value(false)
          .implicit_value(true);

        parser.parse_args(argc, argv);

        in_path = std::filesystem::path(parser.get("input_file"));
        bf_path = std::filesystem::path(parser.get("-b"));
        model_path = std::filesystem::path(parser.get("-m"));
        out_path = std::filesystem::path(parser.get("-o"));
        num_threads = parser.get<unsigned>("-t");
        contig_mode = parser.get<bool>("--contig-mode");
        no_apply = parser.get<bool>("--no-apply");
        verbose = parser.get<bool>("--verbose");
    }
};
