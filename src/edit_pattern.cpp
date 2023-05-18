#include "edit_pattern.hpp"

namespace aiedit {

void EditPattern::set(size_t index, Edit::Type value) { values[index] = value; }

Edit::Type EditPattern::get(size_t index) const { return values[index]; }

size_t EditPattern::get_length() const { return length; }

std::string EditPattern::to_string() const
{
    std::string s = "";
    for (size_t i = 0; i < length; i++) {
        s += (char)values[i];
    }
    return s;
}

}  // namespace aiedit