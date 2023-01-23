#ifndef AIEDIT_EDIT_PATTERN_HPP
#define AIEDIT_EDIT_PATTERN_HPP

#include <cstddef>
#include <string>
#include <vector>

namespace aiedit {

class EditPattern
{
public:
  enum Value
  {
    NONE = '-',
    MISMATCH = 'M',
    INSERTION = 'I',
    DELETION = 'D'
  };

  EditPattern(size_t length)
    : values(new Value[length])
    , length(length)
  {}

  /**
   * Update the edit pattern array
   * @param index Position to be updated
   * @param value New value for the index
   */
  void set(size_t index, Value value);

  /**
   * Get the edit type in an index
   * @param index Position to get
   * @return Value in the index
   */
  Value get(size_t index);

  /**
   * Get a string representation of the edit pattern
   * @return Value array as a string
   */
  std::string to_string() const;

  /**
   * Get a vector of position that require edits
   * @return Vector of position with non-NONE values
   */
  std::vector<size_t> get_edit_positions();

private:
  Value* values;
  const size_t length;
};

};

#endif // AIEDIT_EDIT_PATTERN_HPP