#include "aiedit/signature.hpp"

namespace aiedit {

void
Signature::set(size_t position, unsigned seed_index, bool has_miss)
{
    values[0][position][seed_index] = has_miss ? 0.0 : 1.0;
}

bool
Signature::has_miss(size_t position, unsigned seed_index)
{
    return values[0][position][seed_index].item<float>() == 0.0;
}

std::vector<std::string>
Signature::to_string_vector()
{
    std::vector<std::string> rows;
    for (size_t i = 0; i < get_length(); i++) {
        std::string row;
        for (size_t j = 0; j < get_num_seeds(); j++) {
            row.append(has_miss(i, j) ? "X" : "-");
        }
        rows.emplace_back(row);
    }
    return rows;
}

size_t
Signature::get_length()
{
    return values.sizes()[1];
}

size_t
Signature::get_num_seeds()
{
    return values.sizes()[2];
}

}