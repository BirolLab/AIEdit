#pragma once

#include <cstddef>
#include <deque>
#include <string>
#include <vector>

namespace aiedit {

class Editor
{

  public:

    class Iterator;

    Editor(const std::string_view seq, size_t start_base_position, size_t end_base_position);
    Editor(const Editor& other);

    void substitute(char new_base);
    void insert(char base);
    void delete_base();
    void skip();

    size_t get_num_remaining() const;
    char get_current() const;
    size_t get_size() const;

    Editor::Iterator begin() const;
    Editor::Iterator end() const;

  private:

    const std::string_view seq;
    const size_t end_pos;
    char current;
    size_t pos;
    std::string consumed;
};

class Editor::Iterator
{

  public:

    char operator*() const;
    Iterator& operator++();
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

  private:

    const Editor& editor;
    size_t index;

    Iterator(const Editor& editor, size_t index);

    friend class Editor;
};

}