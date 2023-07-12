#ifndef AIEDIT_EDIT_HPP
#define AIEDIT_EDIT_HPP

#include <cstddef>
#include <string>

namespace aiedit {

class Edit
{
  public:

    enum Type
    {
        NONE = '-',
        MISMATCH = 'M',
        INSERTION = 'I',
        DELETION = 'D'
    };

    Edit(size_t position, Type type, char before, char after)
      : position(position)
      , type(type)
      , before(before)
      , after(after)
    {}

    size_t get_position() const { return position; }
    Type get_type() const { return type; }
    char get_before() const { return before; }
    char get_after() const { return after; }

  private:

    size_t position;
    Type type;
    char before;
    char after;
};

}  // namespace aiedit

#endif  // AIEDIT_EDIT_HPP