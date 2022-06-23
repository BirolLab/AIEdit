#ifndef AIEDIT_LOGGING_HPP
#define AIEDIT_LOGGING_HPP

#include <chrono>
#include <string>

enum Verbosity
{
  NONE,
  NORMAL,
  DETAILED
};

class Logger
{
private:
  const Verbosity verbosity;

public:
  explicit Logger(const Verbosity& verbosity)
    : verbosity(verbosity)
  {}

  void print(const std::string& message,
             const Verbosity& level = Verbosity::NORMAL,
             const std::string& endl = "\n");
};

class Timer
{
private:
  std::clock_t t_start;
  std::clock_t t_end;

public:
  void start();
  void stop();
  [[nodiscard]] long double elapsed_seconds() const;
  [[nodiscard]] std::string to_string() const;
};

#endif // AIEDIT_LOGGING_HPP
