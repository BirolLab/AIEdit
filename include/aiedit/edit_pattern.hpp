#ifndef AIEDIT_EDIT_PATTERN_HPP
#define AIEDIT_EDIT_PATTERN_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "aiedit/edit.hpp"

namespace aiedit {

class EditPattern
{
  public:

    EditPattern(size_t length) : values(new Edit::Type[length]), length(length) {}

    /**
     * Update the edit pattern array
     * @param index Position to be updated
     * @param value New value for the index
     */
    void set(size_t index, Edit::Type value);

    /**
     * Get the edit type in an index
     * @param index Position to get
     * @return Value in the index
     */
    Edit::Type get(size_t index) const;

    /**
     * Get pattern length
     * @return Length of the edit pattern
     */
    size_t get_length() const;

    /**
     * Get a string representation of the edit pattern
     * @return Value array as a string
     */
    std::string to_string() const;

  private:

    std::shared_ptr<Edit::Type[]> values;
    const size_t length;
};

}  // namespace aiedit

#endif  // AIEDIT_EDIT_PATTERN_HPP