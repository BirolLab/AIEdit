#include "edit.hpp"
#include <utility>

namespace aiedit {

Edit::Edit(size_t pos, char before, char after)
  : pos(pos)
  , before(before)
  , after(after)
{}

Edit& Edit::operator=(Edit other)
{
    std::swap(const_cast<size_t&>(pos), const_cast<size_t&>(other.pos));
    std::swap(const_cast<char&>(before), const_cast<char&>(other.before));
    std::swap(const_cast<char&>(after), const_cast<char&>(other.after));
    return *this;
}

const Edit Edit::substitution(size_t pos, char before, char after)
{
    return Edit(pos, before, after);
}

const Edit Edit::deletion(size_t pos, char deleted) { return Edit(pos, deleted, Edit::NO_BASE); }

const Edit Edit::insertion(size_t pos, char inserted) { return Edit(pos, Edit::NO_BASE, inserted); }

Edit::Type Edit::get_type() const
{
    if (before != Edit::NO_BASE && after != Edit::NO_BASE) {
        return Edit::Type::SUBSTITUION;
    }
    if (before != Edit::NO_BASE && after == Edit::NO_BASE) {
        return Edit::Type::DELETION;
    }
    if (before == Edit::NO_BASE && after != Edit::NO_BASE) {
        return Edit::Type::INSERTION;
    }
    return Edit::Type::NO_EDIT;
}

}