#include <btllib/bloom_filter.hpp>
#include <btllib/nthash.hpp>
#include <btllib/seq_reader.hpp>
#include <filesystem>
#include <fstream>

#include "data_types.hpp"
#include "error_detection.hpp"
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
  std::filesystem::path out_dir_path(args.out_path);
  ai_edit::print_logo();
  ai_edit::Timer timer;

  // LOAD THE BLOOM FILTER
  std::cout << "Loading Bloom filter... ";
  BENCHMARK(btllib::SeedBloomFilter bloom_filter(args.bloom_filter_path);)
  ai_edit::print_bloom_filter_information(bloom_filter, args.bloom_filter_path);
  std::cout << std::endl;

  // POPULATE PATTERN DATABASE
  std::cout << "Populating pattern database... ";
  BENCHMARK(auto database = ai_edit::build_database(bloom_filter.get_seeds(),
                                                    args.pattern_length,
                                                    args.signature_length);)
  std::string db_file_path = out_dir_path / std::filesystem::path("db.json");
  std::cout << "Saving database to " << db_file_path << "... ";
  BENCHMARK(std::ofstream db_file(db_file_path);
            auto db_json = ai_edit::to_json(database,
                                            args.signature_length,
                                            bloom_filter.get_seeds().size(),
                                            args.pattern_length);
            db_file << db_json.dump(4);
            db_file.flush();)
  std::cout << std::endl;

  // PREPARE SEQ READER
  unsigned seq_reader_flags;
  if (args.seq_reader_long_mode) {
    seq_reader_flags = btllib::SeqReader::Flag::LONG_MODE;
    std::cout << "SeqReader long read optimizations (--long-mode)  = yes"
              << std::endl;
  } else {
    seq_reader_flags = btllib::SeqReader::Flag::SHORT_MODE;
    std::cout << "SeqReader long read optimizations (--long-mode)  = no"
              << std::endl;
  }
  btllib::SeqReader reader(args.assembly_path, seq_reader_flags);

  // EDITING PROCEDURE
  std::cout << "Assembly file    (-a)  = " << args.assembly_path << std::endl;
  std::cout << "Signature length (-n)  = " << args.signature_length
            << std::endl;
  std::cout << "Pattern length   (-w)  = " << args.pattern_length << std::endl;
  std::cout << "Detecting and polishing errors... ";
  timer.start();
  std::ofstream out_tsv(out_dir_path / std::filesystem::path("mismatches.tsv"));
  auto signature = ai_edit::create_signature(args.signature_length,
                                             bloom_filter.get_seeds().size());
  uint64_t num_patterns = 0;
  for (const auto& record : reader) {
    btllib::SeedNtHash hash_function(record.seq,
                                     bloom_filter.get_seeds(),
                                     bloom_filter.get_hash_num_per_seed(),
                                     bloom_filter.get_k());
    while (ai_edit::find_next_miss(hash_function, bloom_filter)) {
      auto miss_position = hash_function.get_pos() + bloom_filter.get_k();
      ai_edit::update_signature(
        hash_function, bloom_filter, signature, args.signature_length);
      auto query_result = ai_edit::query(signature,
                                         args.signature_length,
                                         bloom_filter.get_seeds().size(),
                                         database);
      auto pattern = query_result.entry.pattern;
      out_tsv << "\"" << record.id << "\"\t" << miss_position << "\t"
              << ai_edit::to_string(pattern, args.pattern_length) << "\t"
              << query_result.distance << std::endl;
      ++num_patterns;
    }
  }
  timer.stop();
  timer.print_done();
  std::cout << "- Number of detected patterns = " << num_patterns << std::endl;
  std::cout << "Results saved to " << args.out_path << std::endl;

  return 0;
}