#include <btllib/bloom_filter.hpp>
#include <btllib/nthash.hpp>
#include <btllib/seq_reader.hpp>
#include <filesystem>
#include <fstream>

#include "data_types.hpp"
#include "error_detection.hpp"
#include "file_management.hpp"
#include "pattern_database.hpp"
#include "user_interface.hpp"

#define BENCHMARK(CODE)                                                        \
  timer.start();                                                               \
  CODE timer.stop();                                                           \
  timer.print_done();

int
main(int argc, char** argv)
{
  auto args = ai_edit::parse_args(argc, argv);
  ai_edit::Timer timer;

  ai_edit::print_logo();
  ai_edit::print_args(args);
  std::cout << std::endl;

  // LOAD THE BLOOM FILTER
  std::cout << "Loading Bloom filter... ";
  BENCHMARK(btllib::SeedBloomFilter bloom_filter(args.bloom_filter_path);)
  if (args.verbose) {
    ai_edit::print_bloom_filter_information(bloom_filter,
                                            args.bloom_filter_path);
    std::cout << std::endl;
  }

  // POPULATE PATTERN DATABASE
  std::cout << "Populating pattern database... ";
  BENCHMARK(auto database = ai_edit::build_database(bloom_filter.get_seeds(),
                                                    args.pattern_length,
                                                    args.signature_length);)
  std::cout << "Saving database... ";
  std::string db_file_path = args.out_path / std::filesystem::path("db.json");
  BENCHMARK(ai_edit::write_database_file(database,
                                         args.signature_length,
                                         bloom_filter.get_seeds().size(),
                                         args.pattern_length,
                                         db_file_path);)
  if (args.verbose) {
    std::cout << std::endl;
  }

  // PREPARE FOR EDITING
  unsigned seq_reader_flags;
  if (args.seq_reader_long_mode) {
    seq_reader_flags = btllib::SeqReader::Flag::LONG_MODE;
  } else {
    seq_reader_flags = btllib::SeqReader::Flag::SHORT_MODE;
  }
  btllib::SeqReader reader(args.assembly_path, seq_reader_flags);
  auto edits_file_path = args.out_path / std::filesystem::path("edits.tsv");
  ai_edit::EditsFile edits_file(edits_file_path);
  auto signature = ai_edit::create_signature(args.signature_length,
                                             bloom_filter.get_seeds().size());

  // EDITING PROCEDURE
  ai_edit::EditingLog edit_log;
  std::cout << "Detecting errors and editing assembly... ";
  timer.start();
  for (auto record : reader) {
    std::string& seq = record.seq;
    btllib::SeedNtHash hash_function(seq,
                                     bloom_filter.get_seeds(),
                                     bloom_filter.get_hash_num_per_seed(),
                                     bloom_filter.get_k());
    while (ai_edit::roll_to_next_miss(hash_function, bloom_filter)) {
      ai_edit::update_signature(hash_function,
                                bloom_filter,
                                signature,
                                args.signature_length);
      auto query_result = ai_edit::query(signature,
                                         args.signature_length,
                                         bloom_filter.get_seeds().size(),
                                         database);
      auto pattern = query_result.entry.pattern;
      edits_file.write(record.id,
                       hash_function.get_pos() + hash_function.get_k(),
                       ai_edit::pattern_to_string(pattern, args.pattern_length),
                       query_result.distance);
      ++edit_log.num_patterns;
    }
  }
  timer.stop();
  timer.print_done();

  if (args.verbose) {
    ai_edit::print_editing_log(edit_log);
    std::cout << std::endl;
  }

  std::cout << "Results saved to " << args.out_path << std::endl;
  if (args.verbose) {
    ai_edit::print_output_files_list();
  }

  return 0;
}