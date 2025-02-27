#pragma once

#include <deque>
#include <string>
#include <vector>

namespace aiedit {

class RegionEditor
{

  public:

    class Iterator;

    RegionEditor(const std::string_view seq, size_t start, size_t end);

    void substitute(char new_base);
    void insert(char base);
    void delete_base();
    void skip();

    size_t get_position() const;
    char get_current() const;
    size_t get_size() const;

    RegionEditor::Iterator begin() const;
    RegionEditor::Iterator end() const;

    const std::vector<char>& get_consumed() const;

  private:

    const size_t end_pos;
    std::vector<char> consumed;
    std::deque<char> nexts;
};

class RegionEditor::Iterator
{

  public:

    char operator*() const;
    Iterator& operator++();
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

  private:

    const RegionEditor& editor;
    size_t index;

    Iterator(const RegionEditor& editor, size_t index);

    friend class RegionEditor;
};

}