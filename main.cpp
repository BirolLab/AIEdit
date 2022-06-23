#include "AIEdit/database.hpp"
#include "AIEdit/observer.hpp"
#include "AIEdit/utils.hpp"
#include "args.hpp"
#include "logging.hpp"
#include <btllib/bloom_filter.hpp>
#include <btllib/seq_reader.hpp>
#include <fstream>

int
main(int argc, char** argv)
{
  auto args = ProgramArguments::get_instance();
  args.parse(argc, argv);

  Logger logger(args.get_verbosity());
  Timer timer{};

  logger.print("Populating database... ", Verbosity::NORMAL, "");
  timer.start();
  const unsigned w = args.get_window_size(), n = args.get_num_frames();
  ai_edit::PatternDatabase db(w, n, args.get_seeds());
  timer.stop();
  logger.print("DONE (" + timer.to_string() + ")");
  logger.print("Database dump:", Verbosity::DETAILED);
  logger.print(db.to_string(), Verbosity::DETAILED);

  if (!args.get_db_path().empty() && ai_edit::file_exists(args.get_db_path())) {
    std::string msg = "Saving database (" + args.get_db_path() + ")... ";
    logger.print(msg, Verbosity::NORMAL, "");
    timer.start();
    std::ofstream db_json_file(args.get_db_path());
    db_json_file << db.to_json();
    db_json_file.flush();
    timer.stop();
    logger.print("DONE (" + timer.to_string() + ")");
  }

  logger.print("Loading Bloom filter... ", Verbosity::NORMAL, "");
  timer.start();
  btllib::BloomFilter bf(args.get_filter_path());
  timer.stop();
  logger.print("DONE (" + timer.to_string() + ")");

  unsigned seq_reader_flags;
  if (args.is_long_mode()) {
    seq_reader_flags = btllib::SeqReader::Flag::LONG_MODE;
  } else {
    seq_reader_flags = btllib::SeqReader::Flag::SHORT_MODE;
  }
  btllib::SeqReader reader(args.get_input_path(), seq_reader_flags);
  std::ofstream out("out.txt");
  for (const auto& record : reader) {
    logger.print("Working on " + record.id + "... ", Verbosity::DETAILED, "");
    timer.start();
    ai_edit::Observer observer(
      record.seq, args.get_seeds(), args.get_num_frames(), bf);
    while (observer.next()) {
      unsigned d;
      ai_edit::Pattern q = db.query(observer.get_current_pattern(), d);
      out << observer.get_position() << "\t" << ai_edit::bool_vec_to_str(q)
          << "\t" << d << std::endl;
    }
    timer.stop();
    logger.print("DONE (" + timer.to_string() + ")", Verbosity::DETAILED);
  }

  return 0;
}
