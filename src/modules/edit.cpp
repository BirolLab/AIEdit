#include "edit.hpp"

namespace aiedit {

Edit::Edit(size_t position, Type type, char new_base)
  : position(position)
  , type(type)
  , new_base(new_base)
{}

Edit Edit::substitution(size_t position, char new_base)
{
    return Edit(position, Edit::Type::SUBSTITUTE, new_base);
}

Edit Edit::insertion(size_t position, char base)
{
    return Edit(position, Edit::Type::INSERT, base);
}

Edit Edit::deletion(size_t position)
{
    return Edit(position, Edit::Type::DELETE, Edit::NO_BASE);
}

}
