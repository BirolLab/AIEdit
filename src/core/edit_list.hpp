#pragma once

#include <atomic>
#include <string>
#include <vector>

#include "edit.hpp"
#include "thread_safe_queue.hpp"

namespace aiedit {

class EditList : public ThreadSafeQueue<Edit>
{

  public:

    EditList();

    void add(Edit edit);

    size_t get_num_passed() const;

    void sort();

    std::string apply(const std::string_view seq);

    std::deque<Edit>::const_iterator begin() const;
    std::deque<Edit>::const_iterator end() const;

  private:

    std::atomic_size_t num_passed;
};

}