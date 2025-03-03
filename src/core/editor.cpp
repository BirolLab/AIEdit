#include "editor.hpp"

#include <stdexcept>

namespace aiedit {

// TODO optimize by avoiding copying in nexts

Editor::Editor(const std::string_view seq, size_t start, size_t end)
  : end_pos(end)
  , nexts(seq.begin() + start, seq.begin() + end)
{}

void Editor::substitute(char new_base)
{
    if (nexts.size() == 0) {
        throw std::runtime_error("[aiedit::Editor] No bases to substitute");
    }
    consumed.push_back(new_base);
    nexts.pop_front();
}

void Editor::insert(char base) { consumed.push_back(base); }

void Editor::delete_base()
{
    if (nexts.size() == 0) {
        throw std::runtime_error("[aiedit::Editor] No bases to delete");
    }
    nexts.pop_front();
}

void Editor::skip()
{
    if (nexts.size() == 0) {
        throw std::runtime_error("[aiedit::Editor] No bases to skip");
    }
    consumed.push_back(nexts.front());
    nexts.pop_front();
}

size_t Editor::get_num_remaining() const { return nexts.size(); }

size_t Editor::get_position() const { return end_pos - nexts.size(); }

char Editor::get_current() const { return nexts.front(); }

size_t Editor::get_size() const { return consumed.size() + nexts.size(); }

Editor::Iterator Editor::begin() const { return Editor::Iterator(*this, 0); }

Editor::Iterator Editor::end() const { return Editor::Iterator(*this, get_size()); }

const std::vector<char>& Editor::get_consumed() const { return consumed; }

Editor::Iterator::Iterator(const Editor& editor, size_t index)
  : editor(editor)
  , index(index)
{}

char Editor::Iterator::operator*() const
{
    if (index < editor.consumed.size()) {
        return editor.consumed[index];
    } else {
        return editor.nexts[index - editor.consumed.size()];
    }
}

Editor::Iterator& Editor::Iterator::operator++()
{
    ++index;
    return *this;
}

bool Editor::Iterator::operator==(const Editor::Iterator& other) const
{
    return index == other.index;
}

bool Editor::Iterator::operator!=(const Editor::Iterator& other) const
{
    return index != other.index;
}

}