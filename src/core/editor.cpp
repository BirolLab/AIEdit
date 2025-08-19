#include "editor.hpp"

#include <stdexcept>

namespace aiedit {

Editor::Editor(const std::string_view seq, size_t start, size_t end)
  : seq(seq)
  , end_pos(end)
  , current(seq[start])
  , pos(start + 1)
{}

Editor::Editor(const Editor& other)
  : seq(other.seq)
  , end_pos(other.end_pos)
  , current(other.current)
  , pos(other.pos)
  , consumed(other.consumed)
{}

void Editor::substitute(char new_base)
{
    if (get_num_remaining() == 0) {
        throw std::runtime_error("[aiedit::Editor] No bases to substitute");
    }
    current = new_base;
}

void Editor::insert(char base) { consumed.push_back(base); }

void Editor::delete_base()
{
    if (get_num_remaining() == 0) {
        throw std::runtime_error("[aiedit::Editor] No bases to delete");
    }
    current = seq[pos++];
}

void Editor::skip()
{
    if (get_num_remaining() == 0) {
        throw std::runtime_error("[aiedit::Editor] No bases to skip");
    }
    consumed.push_back(current);
    current = seq[pos++];
}

size_t Editor::get_num_remaining() const { return pos <= end_pos ? end_pos - pos + 1 : 0; }

char Editor::get_current() const { return current; }

size_t Editor::get_size() const { return consumed.size() + get_num_remaining(); }

Editor::Iterator Editor::begin() const { return Editor::Iterator(*this, 0); }

Editor::Iterator Editor::end() const { return Editor::Iterator(*this, get_size()); }

Editor::Iterator::Iterator(const Editor& editor, size_t index)
  : editor(editor)
  , index(index)
{}

char Editor::Iterator::operator*() const
{
    if (index < editor.consumed.size()) {
        return editor.consumed[index];
    } else if (index == editor.consumed.size()) {
        return editor.current;
    } else {
        return editor.seq[index - editor.consumed.size() + editor.pos - 1];
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