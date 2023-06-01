#include "pattern.hpp"

namespace aiedit {

void Pattern::set(ptrdiff_t index, Edit::Type value)
{
    --counts[values[index]];
    ++counts[value];
    values[index] = value;
}

Edit::Type Pattern::get(ptrdiff_t index) const { return values[index]; }

size_t Pattern::get_length() const { return length; }

unsigned Pattern::get_count(Edit::Type type) { return counts[type]; }

std::string Pattern::to_string() const
{
    std::string s;
    for (unsigned i = 0; i < length; i++) {
        s += (char)values[i];
    }
    return s;
}

}  // namespace aiedit