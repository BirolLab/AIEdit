#include <btllib/bloom_filter.hpp>
#include <btllib/nthash.hpp>
#include <btllib/seq_reader.hpp>
#include <btllib/seq_writer.hpp>
#include <filesystem>
#include <fstream>
#include <progressbar.hpp>

#include "data_types.hpp"
#include "editing.hpp"
#include "error_detection.hpp"
#include "file_management.hpp"
#include "nthash/nthash.hpp"
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
  progressbar pbar(100, args.verbosity < 2);
  pbar.set_todo_char(" ");
  pbar.set_done_char("\033[1;34m█\033[0m");
  pbar.set_opening_bracket_char("[");
  pbar.set_closing_bracket_char("]");

  ai_edit::print_logo();
  ai_edit::print_args(args);
  std::cout << std::endl;

  // LOAD THE BLOOM FILTER
  std::cout << "Loading Bloom filter... " << std::flush;
  BENCHMARK(btllib::SeedBloomFilter bloom_filter(args.bloom_filter_path);)
  if (args.verbosity > 0) {
    ai_edit::print_bloom_filter_information(bloom_filter,
                                            args.bloom_filter_path);
    std::cout << std::endl;
  }

  // POPULATE PATTERN DATABASE
  std::cout << "Populating pattern database... " << std::flush;
  BENCHMARK(auto database = ai_edit::build_database(bloom_filter.get_seeds(),
                                                    args.pattern_length,
                                                    args.signature_length);)
  std::cout << "Saving database... " << std::flush;
  std::string db_file_path = args.out_path / std::filesystem::path("db.json");
  BENCHMARK(ai_edit::write_database_file(database,
                                         args.signature_length,
                                         bloom_filter.get_seeds().size(),
                                         args.pattern_length,
                                         db_file_path);)
  if (args.verbosity > 0) {
    ai_edit::print_database_information(database,
                                        args.signature_length,
                                        args.pattern_length);
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
  auto edited_file_path = args.out_path / std::filesystem::path("edited.fa");
  btllib::SeqWriter writer(edited_file_path, btllib::SeqWriter::FASTA);
  auto edits_file_path = args.out_path / std::filesystem::path("edits.tsv");
  ai_edit::EditsFile edits_file(edits_file_path);
  auto vcf_file_path = args.out_path / std::filesystem::path("variants.vcf");
  ai_edit::VCFWriter vcf_file(vcf_file_path, args.assembly_path);
  auto signature = ai_edit::create_signature(args.signature_length,
                                             bloom_filter.get_seeds().size());

  // EDITING PROCEDURE
  ai_edit::EditingLog edit_log;
  size_t file_size = ai_edit::get_file_size(args.assembly_path);
  unsigned percentage = 0;
  std::cout << "Detecting errors and editing assembly... " << std::endl;
  timer.start();
  for (auto record : reader) {
    std::string& seq = record.seq;
    nthash::SeedNtHash hash_function(seq,
                                     bloom_filter.get_seeds(),
                                     bloom_filter.get_hash_num_per_seed(),
                                     bloom_filter.get_k());
    while (ai_edit::roll_to_next_miss(hash_function, bloom_filter)) {
      size_t miss_pos = hash_function.get_pos() + hash_function.get_k() - 1;
      if (args.verbosity > 1) {
        std::cout << "[" << record.id << "] Miss at " << miss_pos + 1 << ": "
                  << std::flush;
      }
      ai_edit::update_signature(hash_function,
                                bloom_filter,
                                signature,
                                args.signature_length);
      auto query_result = ai_edit::query(signature,
                                         args.signature_length,
                                         bloom_filter.get_seeds().size(),
                                         database);
      auto pattern = query_result.entry.pattern;
      auto edits = ai_edit::get_edits(seq,
                                      miss_pos,
                                      pattern,
                                      args.pattern_length,
                                      bloom_filter,
                                      hash_function,
                                      args.signature_length);
      if (edits.size() > 0) {
        edits_file.write(seq,
                         record.id,
                         miss_pos,
                         pattern,
                         args.pattern_length,
                         edits,
                         query_result.distance);
        vcf_file.write(seq, record.id, edits);
        ai_edit::apply_edits(seq, hash_function, edits);
        ++edit_log.num_patterns;
        edit_log.num_edits += edits.size();
        if (args.verbosity > 1) {
          std::cout << "Edited " << edits.size() << " base(s) with pattern "
                    << ai_edit::pattern_to_string(pattern, args.pattern_length)
                    << std::endl;
        }
      } else {
        unsigned rolls = hash_function.get_k() + args.pattern_length;
        for (unsigned i = 0; i < rolls; i++) {
          hash_function.roll();
        }
        if (args.verbosity > 1) {
          std::cout << "No edit pattern detected, skipped " << rolls << " bases"
                    << std::endl;
        }
      }
      size_t bytes_done = record.id.size() + record.comment.size() + miss_pos;
      double percent_done = (double)bytes_done / (double)file_size * 100.0;
      if ((unsigned)(percent_done) > percentage) {
        percentage = (unsigned)(percent_done);
        pbar.update();
      }
    }
    writer.write(record.id, record.comment, seq);
  }
  timer.stop();
  for (unsigned i = 0; i < 100 - percentage; i++) {
    pbar.update();
  }
  std::cout << std::endl;
  timer.print_done();

  if (args.verbosity > 0) {
    ai_edit::print_editing_log(edit_log);
    std::cout << std::endl;
  }

  std::cout << "Results saved to " << args.out_path << std::endl;
  if (args.verbosity > 0) {
    ai_edit::print_output_files_list();
  }

  return 0;
}