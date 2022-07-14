#include "logging.hpp"
#include <iostream>

void
Logger::normal(const std::string& message, bool endl)
{
  if (this->verbosity >= Verbosity::NORMAL) {
    std::cout << message << (endl ? "\n" : "") << std::flush;
  }
}

void
Logger::detailed(const std::string& message, bool endl)
{
  if (this->verbosity >= Verbosity::DETAILED) {
    std::cout << message << (endl ? "\n" : "") << std::flush;
  }
}

void
Timer::start()
{
  this->t_start = clock();
}

void
Timer::stop()
{
  this->t_end = clock();
}

long double
Timer::elapsed_seconds() const
{
  return (long double)(this->t_end - this->t_start) / CLOCKS_PER_SEC;
}

std::string
Timer::to_string() const
{
  return std::to_string(this->elapsed_seconds()) + "s";
}