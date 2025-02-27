#pragma once

#include <cstddef>

namespace aiedit {

class Edit
{
  public:

    enum class Type
    {
        SUBSTITUTE,
        INSERT,
        DELETE,
    };

    const size_t position;
    const Type type;
    const char new_base;

    static constexpr char NO_BASE = '.';
    static Edit substitution(size_t position, char new_base);
    static Edit insertion(size_t position, char base);
    static Edit deletion(size_t position);

  private:

    Edit(size_t position, Type type, char new_base);
};

}