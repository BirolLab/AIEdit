#include "edit_pattern.hpp"

namespace aiedit {

void EditPattern::set(ptrdiff_t index, Edit::Type value) { values[index] = value; }

Edit::Type EditPattern::get(ptrdiff_t index) const { return values[index]; }

size_t EditPattern::get_length() const { return length; }

std::string EditPattern::to_string() const
{
    std::string s;
    for (unsigned i = 0; i < length; i++) {
        s += (char)values[i];
    }
    return s;
}

}  // namespace aiedit