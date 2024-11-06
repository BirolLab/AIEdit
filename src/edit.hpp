#pragma once

#include <cstddef>
#include <string>

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

    Edit(size_t position, Type type, const std::string& before, const std::string& after)
      : position(position)
      , type(type)
      , before(before)
      , after(after)
    {}

    size_t get_position() const { return position; }
    Type get_type() const { return type; }
    const std::string& get_before() const { return before; }
    const std::string& get_after() const { return after; }

  private:

    size_t position;
    Type type;
    std::string before;
    std::string after;
};