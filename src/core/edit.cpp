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

std::string apply_edits(const std::string& seq, const std::vector<Edit>& edits)
{
    std::string edited;
    edited.reserve(seq.size());
    size_t seq_index = 0;
    size_t edit_index = 0;
    while (seq_index < seq.size() || edit_index < edits.size()) {
        if (edit_index < edits.size() && edits[edit_index].pos == seq_index) {
            const auto& edit = edits[edit_index];
            if (edit.get_type() == Edit::Type::INSERTION) {
                edited.push_back(edit.after);
            } else if (edit.get_type() == Edit::Type::DELETION) {
                seq_index++;
            } else if (edit.get_type() == Edit::Type::SUBSTITUION) {
                edited.push_back(edit.after);
                seq_index++;
            }
            edit_index++;
        } else {
            if (seq_index < seq.size()) {
                edited.push_back(seq[seq_index]);
                seq_index++;
            }
        }
    }
    return edited;
}

}