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
    const std::string reference;
    const std::string updated;

    Edit(size_t position, Type type, std::string reference, std::string updated)
      : position(position)
      , type(type)
      , reference(reference)
      , updated(updated)
    {}
};

}

#endif // AIEDIT_EDIT_HPP