#include "aiedit/signature.hpp"

namespace aiedit {

void
Signature::set(size_t position, unsigned seed_index, bool has_miss)
{
    is_miss[position][seed_index] = has_miss;
}

bool
Signature::has_miss(size_t position, unsigned seed_index)
{
    return is_miss[position][seed_index];
}

std::vector<std::string>
Signature::to_string_vector()
{
    std::vector<std::string> rows;
    for (size_t i = 0; i < get_length(); i++) {
        std::string row;
        for (size_t j = 0; j < get_num_seeds(); j++) {
            row.append(is_miss[i][j] ? "X" : "-");
        }
        rows.emplace_back(row);
    }
    return rows;
}

}