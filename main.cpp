#include "AIEdit/database.hpp"
#include "AIEdit/observer.hpp"
#include "AIEdit/utils.hpp"
#include "args.hpp"
#include "logging.hpp"
#include <btllib/bloom_filter.hpp>
#include <btllib/seq_reader.hpp>
#include <filesystem>

int
main(int argc, char** argv)
{
  Timer total_time{};
  total_time.start();

  auto args = ProgramArguments::get_instance();
  args.parse(argc, argv);

  const unsigned n = args.get_num_frames();
  const unsigned w = args.get_window_size();

  Logger log(args.get_verbosity());
  Timer timer{};

  log.normal(LOGO, false);
  log.normal(std::string("\tv") + std::string(VERSION) + "\n");

  log.normal("Loading Bloom filter... ", false);
  timer.start();
  btllib::SeedBloomFilter bf(args.get_filter_path());
  timer.stop();
  log.normal("DONE (" + timer.to_string() + ")");
  log.normal("\t- Path            = " + args.get_filter_path());
  log.normal("\t- Size (bytes)    = " + std::to_string(bf.get_bytes()));
  log.normal("\t- Hashes per seed = " +
             std::to_string(bf.get_hash_num_per_seed()));
  log.normal("\t- Number of seeds = " + std::to_string(bf.get_seeds().size()));
  for (const auto& seed : bf.get_seeds()) {
    log.normal("\t\t- " + seed + " (" + std::to_string(seed.size()) + "bps)");
  }
  log.normal("");

  log.normal("Populating database... ", false);
  timer.start();
  ai_edit::PatternDatabase db(w, n, bf.get_seeds());
  timer.stop();
  log.normal("DONE (" + timer.to_string() + ")");
  log.normal("");

  if (!args.get_db_path().empty() && ai_edit::file_exists(args.get_db_path())) {
    std::string msg = "Saving database (" + args.get_db_path() + ")... ";
    log.normal(msg, false);
    timer.start();
    std::ofstream db_json_file(args.get_db_path());
    db_json_file << db.to_json();
    db_json_file.flush();
    timer.stop();
    log.normal("DONE (" + timer.to_string() + ")");
    log.normal("");
  }

  unsigned seq_reader_flags;
  if (args.is_long_mode()) {
    seq_reader_flags = btllib::SeqReader::Flag::LONG_MODE;
    log.normal("SeqReader will optimize for long sequences\n");
  } else {
    seq_reader_flags = btllib::SeqReader::Flag::SHORT_MODE;
    log.normal("SeqReader will optimize for short sequences\n");
  }
  btllib::SeqReader reader(args.get_input_path(), seq_reader_flags);

  std::filesystem::path out_path(args.get_out_path());
  std::filesystem::path tsv_path("mismatches.tsv");
  std::ofstream out_tsv(out_path / tsv_path);
  out_tsv << "SequenceID\tPosition\tPattern\tDistance" << std::endl;

  uint64_t num_edits = 0, num_patterns = 0;
  log.normal("Signature length (-n) = " +
             std::to_string(args.get_num_frames()));
  log.normal("Window size      (-w) = " +
             std::to_string(args.get_window_size()));
  log.normal("Polishing " + args.get_input_path() + "... ", false);
  timer.start();
  for (const auto& record : reader) {
    ai_edit::Observer obs(record.seq, bf, n);
    while (obs.next()) {
      ai_edit::Signature observed = obs.get_signature();
      unsigned position = obs.get_position(), distance;
      ai_edit::DatabaseEntry result = db.query(observed, distance);
      ai_edit::Pattern pattern = result.get_pattern();
      out_tsv << "\"" << record.id << "\"\t" << position << "\t"
              << pattern.to_string() << "\t" << distance << std::endl;
      ++num_patterns;
      num_edits += pattern.get_num_edits();
    }
  }
  timer.stop();
  log.normal("DONE (" + timer.to_string() + ")");
  log.normal("\t- Number of detected patterns = " +
             std::to_string(num_patterns));
  log.normal("\t- Number of edits             = " + std::to_string(num_edits));
  log.normal("Results saved to " + args.get_out_path());
  log.normal("");

  total_time.stop();
  log.normal("Total run time = " + total_time.to_string());

  return 0;
}
