#ifndef AIEDIT_ARGS_HPP
#define AIEDIT_ARGS_HPP

#define PROGRAM "AIEdit"
#define VERSION "0.0.1"
#define DESCRIPTION ""

#include "logging.hpp"
#include <string>
#include <vector>

class ProgramArguments
{

private:
  ProgramArguments() = default;

  std::vector<std::string> seeds;
  std::string asm_path;
  std::string bf_path;
  std::string db_path;
  std::string out_path;
  Verbosity verbosity{};
  unsigned frame_size{};
  unsigned window_size{};
  bool long_mode;

public:
  static ProgramArguments& get_instance()
  {
    static ProgramArguments instance;
    return instance;
  }

  [[nodiscard]] const std::vector<std::string>& get_seeds() const
  {
    return seeds;
  }

  [[nodiscard]] const std::string& get_input_path() const { return asm_path; }
  [[nodiscard]] const std::string& get_filter_path() const { return bf_path; }
  [[nodiscard]] const std::string& get_db_path() const { return db_path; }
  [[nodiscard]] const std::string& get_out_path() const { return out_path; }
  [[nodiscard]] Verbosity get_verbosity() const { return verbosity; }
  [[nodiscard]] unsigned get_num_frames() const { return frame_size; }
  [[nodiscard]] unsigned get_window_size() const { return window_size; }
  [[nodiscard]] bool is_long_mode() const { return long_mode; }

  void parse(int argc, char** argv);
};

#endif // AIEDIT_ARGS_HPP
