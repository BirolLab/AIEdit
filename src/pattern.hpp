#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "edit.hpp"

class Pattern
{
  public:

    Pattern(size_t length)
      : values(std::make_unique<Edit::Type[]>(length))
      , length(length)
    {
        std::fill_n(values.get(), length, Edit::Type::NONE);
        counts[Edit::Type::NONE] = length;
        counts[Edit::Type::MISMATCH] = 0;
        counts[Edit::Type::INSERTION] = 0;
        counts[Edit::Type::DELETION] = 0;
    }

    /**
     * Update the edit pattern array
     * @param index Position to be updated
     * @param value New value for the index
     */
    void set(ptrdiff_t index, Edit::Type value)
    {
        --counts[values[index]];
        ++counts[value];
        values[index] = value;
    }

    /**
     * Get the edit type in an index
     * @param index Position to get
     * @return Value in the index
     */
    Edit::Type get(ptrdiff_t index) const { return values[index]; }

    /**
     * Get pattern length
     * @return Length of the edit pattern
     */
    size_t get_length() const { return length; }

    /**
     * Get a string representation of the edit pattern
     * @return Value array as a string
     */
    std::string to_string() const
    {
        std::string s;
        for (unsigned i = 0; i < length; i++) {
            s += (char)values[i];
        }
        return s;
    }

    /**
     * Count the numbers of a specific edit type
     * @param type Edit type
     * @return Count of `type`
     */
    unsigned get_count(Edit::Type type) { return counts[type]; }

    /**
     * Check if the pattern is empty
     * @return `true` if all elements of the pattern are set to clean
     */
    bool is_empty() { return counts[Edit::Type::NONE] == length; }

  private:

    std::unique_ptr<Edit::Type[]> values;
    const size_t length;
    std::unordered_map<Edit::Type, unsigned> counts;
};
