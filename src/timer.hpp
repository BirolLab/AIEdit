#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <string>

namespace aiedit {

class Timer
{
  private:

    std::chrono::time_point<std::chrono::system_clock> t_start;
    std::chrono::time_point<std::chrono::system_clock> t_end;

  public:

    /**
     * Register the current time as the timer's starting point.
     */
    void start();

    /**
     * Register the current time as the timer's finish point.
     */
    void stop();

    /**
     * Compute the difference between the start and stop points in seconds.
     */
    [[nodiscard]] long double elapsed_seconds() const;

    /**
     * Get a human-readable representation of the elapsed time.
     */
    [[nodiscard]] std::string to_string() const;
};

}  // namespace aiedit

#endif  // TIMER_HPP