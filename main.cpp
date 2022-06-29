#include "AIEdit/database.hpp"
#include "AIEdit/observer.hpp"
#include "AIEdit/utils.hpp"
#include "args.hpp"
#include "logging.hpp"
#include <btllib/bloom_filter.hpp>
#include <btllib/seq_reader.hpp>
#include <sys/stat.h>

unsigned long long
get_file_size(const std::string& file_path)
{
  struct stat64 file_info
  {};
  if (stat64(file_path.data(), &file_info) == 0) {
    return file_info.st_size;
  }
  return 0;
}

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

  logger.print("Polishing " + args.get_input_path());
  unsigned seq_reader_flags;
  if (args.is_long_mode()) {
    seq_reader_flags = btllib::SeqReader::Flag::LONG_MODE;
  } else {
    seq_reader_flags = btllib::SeqReader::Flag::SHORT_MODE;
  }
  btllib::SeqReader reader(args.get_input_path(), seq_reader_flags);
  std::ofstream out("out.txt");
  unsigned long long file_size = get_file_size(args.get_input_path());
  unsigned long long bytes_read = 0;
  uint8_t percentage = 0;
  timer.start();
  for (const auto& record : reader) {
    logger.print("Working on " + record.id + "... ", Verbosity::DETAILED, "");
    ai_edit::Observer observer(
      record.seq, args.get_seeds(), args.get_num_frames(), bf);
    while (observer.next()) {
      unsigned d;
      auto q = db.query(observer.get_current_pattern(), d);
      out << "\"" << record.id << "\"\t" << observer.get_position() << "\t"
          << q.get_pattern().to_string() << "\t" << d
          << std::endl;
      std::cout << "Observed:" << std::endl;
      for (const auto& s : observer.get_current_pattern().to_string_vec()) {
        std::cout << s << std::endl;
      }
      std::cout << "Closest:" << std::endl;
      for (const auto& s : q.get_frame_data().to_string_vec()) {
        std::cout << s << std::endl;
      }
    }
    logger.print("DONE (" + timer.to_string() + ")", Verbosity::DETAILED);
    bytes_read += record.id.size() + record.seq.size();
    auto p = (uint8_t)((double)bytes_read / (double)file_size * 100.0);
    if (p > percentage) {
      percentage = p;
      std::string msg = "Progress: ";
      msg.append(std::to_string(percentage) + "%");
      timer.stop();
      long double elapsed = timer.elapsed_seconds();
      long double eta = (100.0 - percentage) * elapsed / percentage;
      eta /= 60.0;
      msg.append(", Estimated time left: " + std::to_string((int)eta) + "min");
      logger.print(msg, Verbosity::NORMAL, "\r");
    }
  }

  return 0;
}
