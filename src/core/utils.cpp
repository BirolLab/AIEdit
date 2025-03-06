#include "utils.hpp"

namespace aiedit {

std::string apply_edits(const std::string_view seq, const pybind11::list& edits)
{
    if (edits.size() == 0) {
        return std::string{seq};
    }
    std::string edited;
    edited.reserve(seq.size() + seq.size() / 10);
    size_t seq_pos = 0, edit_index = 0;
    auto next_edit = edits[edit_index].cast<pybind11::tuple>();
    while (seq_pos < seq.size()) {
        if (seq_pos != next_edit[0].cast<size_t>() || edit_index >= edits.size()) {
            edited.push_back(seq[seq_pos++]);
            continue;
        }
        const auto edit_type = next_edit[1].cast<std::string>();
        if (edit_type == "sub") {
            edited.push_back(next_edit[2].cast<char>());
            ++seq_pos;
        } else if (edit_type == "ins") {
            edited.push_back(next_edit[2].cast<char>());
        } else if (edit_type == "del") {
            ++seq_pos;
        }
        if (++edit_index < edits.size()) {
            next_edit = edits[edit_index].cast<pybind11::tuple>();
        }
    }
    return edited;
}

}