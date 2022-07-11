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
  auto args = ProgramArguments::get_instance();
  args.parse(argc, argv);

  const unsigned n = args.get_num_frames();
  const unsigned w = args.get_window_size();

  Logger log(args.get_verbosity());
  Timer timer{};

  log.normal(std::to_string(args.get_seeds().size()) + " seed(s):");
  for (const auto& seed : args.get_seeds()) {
    log.normal(seed + " (" + std::to_string(seed.size()) + "bps)");
  }

  log.normal("Populating database... ", false);
  timer.start();
  ai_edit::PatternDatabase db(w, n, args.get_seeds());
  timer.stop();
  log.normal("DONE (" + timer.to_string() + ")");
  log.detailed("Database dump:");
  log.detailed(db.to_string());

  if (!args.get_db_path().empty() && ai_edit::file_exists(args.get_db_path())) {
    std::string msg = "Saving database (" + args.get_db_path() + ")... ";
    log.normal(msg, false);
    timer.start();
    std::ofstream db_json_file(args.get_db_path());
    db_json_file << db.to_json();
    db_json_file.flush();
    timer.stop();
    log.normal("DONE (" + timer.to_string() + ")");
  }

  log.normal("Loading Bloom filter... ", false);
  timer.start();
  btllib::BloomFilter bf(args.get_filter_path());
  timer.stop();
  log.normal("DONE (" + timer.to_string() + ")");

  unsigned seq_reader_flags;
  if (args.is_long_mode()) {
    seq_reader_flags = btllib::SeqReader::Flag::LONG_MODE;
  } else {
    seq_reader_flags = btllib::SeqReader::Flag::SHORT_MODE;
  }
  btllib::SeqReader reader(args.get_input_path(), seq_reader_flags);

  std::filesystem::path out_path(args.get_out_path());
  std::filesystem::path tsv_path("mismatches.tsv");
  std::ofstream out_tsv(out_path / tsv_path);
  out_tsv << "SequenceID\tPosition\tPattern\tDistance" << std::endl;

  log.normal("Polishing " + args.get_input_path() + "... ", false);
  timer.start();
  for (const auto& record : reader) {
    ai_edit::Observer obs(record.seq, args.get_seeds(), n, bf);
    while (obs.next()) {
      ai_edit::Signature observed = obs.get_signature();
      unsigned position = obs.get_position(), distance;
      ai_edit::DatabaseEntry result = db.query(observed, distance);
      ai_edit::Pattern pattern = result.get_pattern();
      out_tsv << "\"" << record.id << "\"\t" << position << "\t"
              << pattern.to_string() << "\t" << distance << std::endl;
    }
  }
  timer.stop();
  log.normal("DONE (" + timer.to_string() + ")");

  return 0;
}
