#pragma once

#include <cstddef>
#include <string>

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
    Type type;
    std::string edited;
    float score;
    Status status;
};

}