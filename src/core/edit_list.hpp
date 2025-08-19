#pragma once

#include <atomic>
#include <cstddef>
#include <string>
#include <vector>

#include "thread_safe_queue.hpp"

namespace aiedit {

struct Edit {
    enum class Type
    {
        SUBSTITUTE,
        INSERT,
        DELETE,
    };

    enum class Status
    {
        PASS,
        LOW_KMER_SCORE,
        MODEL_FAIL,
    };

    size_t position;
    unsigned num_kmers;
    Type type;
    std::string edited;
    float kmer_score;
    float model_confidence;
    unsigned i_try;
    Status status;
};

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
    std::atomic_int length_diff;
};

}