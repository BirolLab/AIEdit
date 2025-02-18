#include "edit.hpp"

namespace aiedit {

Edit::Edit(size_t position, Type type, char before, char after)
  : position(position)
  , type(type)
  , before(before)
  , after(after)
{}

Edit Edit::substitution(size_t position, char before, char after)
{
    return Edit(position, Edit::Type::SUBSTITUTE, before, after);
}

Edit Edit::insertion(size_t position, char base)
{
    return Edit(position, Edit::Type::INSERT, Edit::NO_BASE, base);
}

Edit Edit::deletion(size_t position, char base)
{
    return Edit(position, Edit::Type::DELETE, base, Edit::NO_BASE);
}

}
