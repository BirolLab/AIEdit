#pragma once

#include <deque>
#include <string>
#include <vector>

namespace aiedit {

class Editor
{

  public:

    class Iterator;

    Editor(const std::string_view seq, size_t start_base_position, size_t end_base_position);

    void substitute(char new_base);
    void insert(char base);
    void delete_base();
    void skip();

    size_t get_num_remaining() const;
    size_t get_position() const;
    char get_current() const;
    size_t get_size() const;

    Editor::Iterator begin() const;
    Editor::Iterator end() const;

    const std::vector<char>& get_consumed() const;

  private:

    const size_t end_pos;
    std::vector<char> consumed;
    std::deque<char> nexts;
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