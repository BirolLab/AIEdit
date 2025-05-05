#include "edit_list.hpp"

#include <algorithm>

namespace aiedit {

EditList::EditList()
  : num_passed(0)
{}

void EditList::push(Edit edit)
{
    ThreadSafeQueue::push(edit);
    if (edit.status == Edit::Status::PASS) {
        ++num_passed;
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
    edited.reserve(seq.size() + seq.size() / 10);
    size_t seq_pos = 0;
    size_t edit_index = 0;
    while (seq_pos < seq.size()) {
        auto& current_edit = items[edit_index];
        if (seq_pos != current_edit.position) {
            edited.push_back(seq[seq_pos++]);
        } else if (current_edit.status != Edit::Status::PASS) {
            edit_index = std::min(edit_index + 1, items.size() - 1);
        } else if (current_edit.type == Edit::Type::SUBSTITUTE) {
            edited.append(current_edit.edited);
            seq_pos += current_edit.edited.size();
        } else if (current_edit.type == Edit::Type::INSERT) {
            edited.append(current_edit.edited);
        } else if (current_edit.type == Edit::Type::DELETE) {
            seq_pos += current_edit.edited.size();
        }
        edit_index = std::min(edit_index + 1, items.size() - 1);
    }
    return edited;
}

std::deque<Edit>::const_iterator EditList::begin() const { return items.begin(); }

std::deque<Edit>::const_iterator EditList::end() const { return items.end(); }

}