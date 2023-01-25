#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <string>

class Timer
{
  private:
    std::clock_t t_start;
    std::clock_t t_end;

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

#endif // TIMER_HPP