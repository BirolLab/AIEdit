#include "timer.hpp"

#include <iostream>

void Timer::start() { this->t_start = std::chrono::system_clock::now(); }

void Timer::stop() { this->t_end = std::chrono::system_clock::now(); }

long double Timer::elapsed_seconds() const
{
    std::chrono::duration<double> elapsed = (t_end - t_start);
    return elapsed.count();
}

std::string Timer::to_string() const { return std::to_string(this->elapsed_seconds()) + " s"; }
