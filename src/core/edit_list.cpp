#include "edit_list.hpp"

#include <algorithm>
#include <iostream>

namespace aiedit {

EditList::EditList()
  : num_passed(0)
  , length_diff(0)
{}

void EditList::add(Edit edit)
{
    ThreadSafeQueue::push(edit);
    if (edit.status == Edit::Status::PASS) {
        ++num_passed;
    }
    if (edit.type == Edit::Type::DELETE) {
        length_diff -= edit.edited.size();
    } else if (edit.type == Edit::Type::INSERT) {
        length_diff += edit.edited.size();
    }
}

size_t EditList::get_num_passed() const { return num_passed; }

void EditList::sort()
{
    std::sort(items.begin(), items.end(), [](const Edit& a, const Edit& b) {
        return a.position < b.position;
    });
}

std::string EditList::apply(const std::string_view seq)
{
    if (items.size() == 0) {
        return std::string{seq};
    }
    std::string edited;
    edited.reserve(seq.size() + length_diff);
    size_t seq_pos = 0;
    size_t edit_index = 0;
    while (seq_pos < seq.size()) {
        if (edit_index < items.size() && items[edit_index].status != Edit::Status::PASS) {
            ++edit_index;
            continue;
        }
        if (edit_index >= items.size() || seq_pos != items[edit_index].position) {
            edited.push_back(seq[seq_pos++]);
            continue;
        }
        if (items[edit_index].type == Edit::Type::SUBSTITUTE) {
            edited.append(items[edit_index].edited);
            seq_pos += items[edit_index].edited.size();
        } else if (items[edit_index].type == Edit::Type::INSERT) {
            edited.append(items[edit_index].edited);
        } else if (items[edit_index].type == Edit::Type::DELETE) {
            seq_pos += items[edit_index].edited.size();
        }
        ++edit_index;
    }
    return edited;
}

std::deque<Edit>::const_iterator EditList::begin() const { return items.begin(); }

std::deque<Edit>::const_iterator EditList::end() const { return items.end(); }

}