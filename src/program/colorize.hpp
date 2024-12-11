#pragma once

#include <sstream>
#include <string>

namespace {

enum Color
{
    FG_RED = 31,
    FG_GREEN = 32,
    FG_BLUE = 34,
    FG_YELLOW = 33,
    FG_DEFAULT = 39,
    BG_RED = 41,
    BG_GREEN = 42,
    BG_BLUE = 44,
    BG_YELLOW = 43,
    BG_DEFAULT = 49
};

template <typename... Args>
std::string add_color(Color color, Args&&... args)
{
    std::ostringstream oss;
    (oss << ... << std::forward<Args>(args));
    return "\033[;" + std::to_string(color) + "m" + oss.str() + "\033[0m";
}

}  // namespace

namespace colorize {

template <typename... Args>
std::string red(Args&&... args)
{
    return add_color(FG_RED, std::forward<Args>(args)...);
}

template <typename... Args>
std::string green(Args&&... args)
{
    return add_color(FG_GREEN, std::forward<Args>(args)...);
}

}  // namespace colorize
