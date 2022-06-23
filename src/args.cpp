#include "args.hpp"
#include "sstream"
#include <argparse/argparse.hpp>

void
ProgramArguments::parse(int argc, char** argv)
{
  auto default_args = argparse::default_arguments::none;
  auto parser = argparse::ArgumentParser(PROGRAM, VERSION, default_args);
  parser.add_description(DESCRIPTION);

  parser.add_argument("--assembly", "-a")
    .help("Path to assembly file")
    .required();

  parser.add_argument("--bloom", "-b")
    .help("Path to btllib-format Bloom filter containing filtered reads")
    .required();

  parser.add_argument("--seeds", "-s")
    .help("Input spaced seeds separated by commas (e.g. 11011,1011011)")
    .required();

  parser.add_argument("--database", "-d")
    .help("Path to load database json file, or save to if file does not exist");

  parser.add_argument("--long-mode")
    .help("Optimize seq. reader for long data (>5kbp)")
    .default_value(false)
    .implicit_value(true);

  parser.add_argument("--num-frames", "-n")
    .help("Number of frames in each pattern")
    .default_value(10U)
    .scan<'u', unsigned>();

  parser.add_argument("--verbosity", "-v")
    .help("Verbosity level (0: none, 1: normal, 2: detailed)")
    .default_value((int)1)
    .scan<'i', int>();

  parser.add_argument("--window-size", "-w")
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

  this->asm_path = parser.get("-a");
  this->bf_path = parser.get("-b");
  this->long_mode = parser.get<bool>("--long-mode");
  this->frame_size = parser.get<unsigned>("-n");
  this->verbosity = Verbosity(parser.get<int>("-v"));
  this->window_size = parser.get<unsigned>("-w");

  if (parser.is_used("-d")) {
    this->db_path = parser.get("-d");
  } else {
    this->db_path = "";
  }

  std::istringstream ss(parser.get("-s"));
  std::string seed_string;
  while (std::getline(ss, seed_string, ',')) {
    this->seeds.push_back(seed_string);
  }
}
