#pragma once

#include <chrono>
#include <string>

class Timer
{
  public:

    void start() { this->t_start = std::chrono::system_clock::now(); }

    [[nodiscard]] double stop()
    {
        const auto t_end = std::chrono::system_clock::now();
        const std::chrono::duration<double> elapsed = (t_end - t_start);
        return elapsed.count();
    }

  private:

    std::chrono::time_point<std::chrono::system_clock> t_start;
};
