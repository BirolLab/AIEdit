#pragma once

#include <cstddef>

namespace aiedit {

class Edit
{
  public:

    static constexpr char NO_BASE = '.';

    enum class Type
    {
        SUBSTITUTE,
        INSERT,
        DELETE,
        NONE
    };

    const size_t position;
    const Type type;
    const char before, after;

    static Edit substitution(size_t position, char before, char after);
    static Edit insertion(size_t position, char base);
    static Edit deletion(size_t position, char base);

  private:

    Edit(size_t position, Type type, char before, char after);
};

}