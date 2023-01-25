#include "timer.hpp"

#include <iostream>

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
