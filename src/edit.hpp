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

    const size_t position;
    const Type type;
    const char before;
    const char after;

    Edit(size_t position, Type type, char before, char after)
      : position(position)
      , type(type)
      , before(before)
      , after(after)
    {}
};

}  // namespace aiedit

#endif  // AIEDIT_EDIT_HPP