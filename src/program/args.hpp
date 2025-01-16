#pragma once

#include <argparse/argparse.hpp>
#include <iostream>
#include <string>

class ProgramArguments
{
  public:

    std::string in_path;
    std::string cbf_path;
    std::string probs_path;
    std::string seeds_path;
    std::string model_path;
    std::string out_path;
    unsigned num_threads;
    bool contig_mode;
    bool no_apply;
    bool train;
    bool verbose;

    void parse(int argc, char** argv)
    {
        argparse::ArgumentParser parser("AIEdit", aiedit::VERSION);

        parser.add_argument("input_file").help("path to input file");

        parser.add_argument("--bloom-filter", "-b")
          .help("path to ntHits counting Bloom filter file")
          .required();

        parser.add_argument("--model", "-m").help("path to pattern detector model").required();

        parser.add_argument("--probabilities", "-p")
          .help("path to count probabilities file")
          .required();

        parser.add_argument("--seeds", "-s").help("path to spaced seeds file").required();

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

        parser.add_argument("--train")
          .help("train model instead of finding edits")
          .default_value(false)
          .implicit_value(true);

        parser.add_argument("--verbose")
          .help("print more details to stdout and log ignored patterns to ignored.tsv")
          .default_value(false)
          .implicit_value(true);

        help_message = parser.help().str();
        parser.parse_args(argc, argv);

        in_path = parser.get("input_file");
        cbf_path = parser.get("-b");
        probs_path = parser.get("-p");
        seeds_path = parser.get("-s");
        model_path = parser.get("-m");
        out_path = parser.get("-o");
        num_threads = parser.get<unsigned>("-t");
        contig_mode = parser.get<bool>("--contig-mode");
        no_apply = parser.get<bool>("--no-apply");
        train = parser.get<bool>("--train");
        verbose = parser.get<bool>("--verbose");
    }

    void print_help(std::ostream& stream) { stream << help_message; }

  private:

    std::string help_message;
};