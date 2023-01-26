#ifndef AIEDIT_EDIT_HPP
#define AIEDIT_EDIT_HPP

#include <stddef.h>

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
    const std::string before;
    const std::string after;

    Edit(size_t position, Type type, std::string before, std::string after)
      : position(position)
      , type(type)
      , before(before)
      , after(after)
    {}
};

}

#endif // AIEDIT_EDIT_HPP