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
    const std::string before;
    const std::string after;

    Edit(size_t position, Type type, std::string before, std::string after)
      : position(position)
      , type(type)
      , before(std::move(before))
      , after(std::move(after))
    {}
};

}  // namespace aiedit

#endif  // AIEDIT_EDIT_HPP