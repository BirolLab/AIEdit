#include "user_interface.hpp"

#include <iostream>

#include <argparse/argparse.hpp>

ai_edit::ProgramArguments
ai_edit::parse_args(int argc, char** argv)
{
  ProgramArguments args;
  auto parser = argparse::ArgumentParser(PROGRAM, VERSION);
  parser.add_description(DESCRIPTION);

  parser.add_argument("--assembly", "-a")
    .help("Path to assembly file")
    .required();

  parser.add_argument("--bloom-filter", "-b")
    .help("Path to btllib SeedBloomFilter populated with reads and seeds")
    .required();

  parser.add_argument("--long-mode")
    .help("Optimize seq. reader for long data (>5kbp)")
    .default_value(false)
    .implicit_value(true);

  parser.add_argument("--out-path", "-o")
    .help("Path to output directory for storing results")
    .default_value(".");

  parser.add_argument("--signature-length", "-n")
    .help("Number of frames in each pattern")
    .default_value(10U)
    .scan<'u', unsigned>();

  parser.add_argument("--verbose")
    .help("Print a more detailed log to stdout")
    .default_value(false)
    .implicit_value(true);

  parser.add_argument("--pattern-length", "-w")
    .help("Number of bases to scan for mismatches")
    .default_value((unsigned)5)
    .scan<'u', unsigned>();

  try {
    parser.parse_args(argc, argv);
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << parser;
    std::exit(1);
  }

  args.assembly_path = std::filesystem::path(parser.get("-a"));
  args.bloom_filter_path = std::filesystem::path(parser.get("-b"));
  args.out_path = std::filesystem::path(parser.get("-o"));
  args.verbose = parser.get<bool>("--verbose");
  args.signature_length = parser.get<unsigned>("-n");
  args.pattern_length = parser.get<unsigned>("-w");
  args.seq_reader_long_mode = parser.get<bool>("--long-mode");

  return args;
}

void
ai_edit::Timer::start()
{
  this->t_start = clock();
}

void
ai_edit::Timer::stop()
{
  this->t_end = clock();
}

long double
ai_edit::Timer::elapsed_seconds() const
{
  return (long double)(this->t_end - this->t_start) / CLOCKS_PER_SEC;
}

std::string
ai_edit::Timer::to_string() const
{
  return std::to_string(this->elapsed_seconds()) + "s";
}

void
ai_edit::Timer::print_done() const
{
  std::cout << "DONE (" << this->to_string() << ")" << std::endl;
}

void
ai_edit::print_logo()
{
  std::cout << LOGO << "\tv" << VERSION << std::endl << std::endl;
}

void
ai_edit::print_bloom_filter_information(const btllib::SeedBloomFilter& bf,
                                        const std::string& path)
{
  std::cout << "- Size (bytes)    = " << bf.get_bytes() << std::endl;
  std::cout << "- FPR             = " << bf.get_fpr() << std::endl;
  std::cout << "- Occupancy       = " << bf.get_occupancy() << std::endl;
  std::cout << "- Hashes per seed = " << bf.get_hash_num_per_seed()
            << std::endl;
  std::cout << "- " << bf.get_seeds().size() << " seed patterns (each "
            << bf.get_k() << "bp long):" << std::endl;
  for (size_t i = 0; i < bf.get_seeds().size(); i++) {
    std::cout << "    - Seed " << i + 1 << ": " << bf.get_seeds()[i]
              << std::endl;
  }
}

void
ai_edit::print_args(const ai_edit::ProgramArguments& args)
{
  std::cout << "Assembly file     (-a)  = " << args.assembly_path << std::endl;
  std::cout << "Bloom filter file (-b)  = " << args.bloom_filter_path
            << std::endl;
  std::cout << "Signature length  (-n)  = " << args.signature_length
            << std::endl;
  std::cout << "Pattern length    (-w)  = " << args.pattern_length << std::endl;
  std::cout << "Optimize for reading long sequences (--long-mode) = "
            << (args.seq_reader_long_mode ? "yes" : "no") << std::endl;
}

void
ai_edit::print_output_files_list()
{}

void
ai_edit::print_editing_log(const ai_edit::EditingLog& log)
{
  std::cout << "- Detected patterns = " << log.num_patterns << std::endl;
}