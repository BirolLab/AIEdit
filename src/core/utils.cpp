#include "utils.hpp"

#include "edit.hpp"

#include <string>

namespace aiedit {

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