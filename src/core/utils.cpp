#include "utils.hpp"

namespace aiedit {

std::string
apply_edits(const std::string_view seq, const pybind11::list& edits, float score_threshold)
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
        const auto edit = next_edit[2].cast<std::string>();
        if (next_edit[3].cast<float>() >= score_threshold) {
            for (size_t i = 0; i < edit.size(); i++) {
                if (edit[i] == '+') {
                    edited.push_back(edit[++i]);
                } else if (edit[i] == '-') {
                    ++seq_pos;
                } else if (edit[i] == '*') {
                    edited.push_back(seq[seq_pos++]);
                } else {
                    edited.push_back(edit[i]);
                    ++seq_pos;
                }
            }
        }
        if (++edit_index < edits.size()) {
            next_edit = edits[edit_index].cast<pybind11::tuple>();
        }
    }
    return edited;
}

}