#include "region_editor.hpp"

namespace aiedit {

RegionEditor::RegionEditor(const std::string_view seq, size_t start, size_t end)
  : end_pos(end)
  , nexts(seq.begin() + start, seq.begin() + end)
{}

void RegionEditor::substitute(char new_base)
{
    consumed.push_back(new_base);
    nexts.pop_front();
}

void RegionEditor::insert(char base) { consumed.push_back(base); }

void RegionEditor::delete_base() { nexts.pop_front(); }

void RegionEditor::skip()
{
    consumed.push_back(nexts.front());
    nexts.pop_front();
}

size_t RegionEditor::get_position() const { return end_pos - nexts.size(); }

char RegionEditor::get_current() const { return nexts.front(); }

size_t RegionEditor::get_size() const { return consumed.size() + nexts.size(); }

RegionEditor::Iterator RegionEditor::begin() const { return RegionEditor::Iterator(*this, 0); }

RegionEditor::Iterator RegionEditor::end() const
{
    return RegionEditor::Iterator(*this, get_size());
}

const std::vector<char>& RegionEditor::get_consumed() const { return consumed; }

RegionEditor::Iterator::Iterator(const RegionEditor& editor, size_t index)
  : editor(editor)
  , index(index)
{}

char RegionEditor::Iterator::operator*() const
{
    if (index < editor.consumed.size()) {
        return editor.consumed[index];
    } else {
        return editor.nexts[index - editor.consumed.size()];
    }
}

RegionEditor::Iterator& RegionEditor::Iterator::operator++()
{
    ++index;
    return *this;
}

bool RegionEditor::Iterator::operator==(const RegionEditor::Iterator& other) const
{
    return index == other.index;
}

bool RegionEditor::Iterator::operator!=(const RegionEditor::Iterator& other) const
{
    return index != other.index;
}

}