#include "aiedit/signature.hpp"

namespace aiedit {

void
Signature::set(size_t position, unsigned seed_index, bool has_miss)
{
    values.set(fdeep::tensor_pos(position, seed_index), has_miss ? 0.0 : 1.0);
}

bool
Signature::has_miss(size_t position, unsigned seed_index)
{
    return values.get(fdeep::tensor_pos(position, seed_index)) == 0.0;
}

size_t
Signature::get_length()
{
    return values.width();
}

size_t
Signature::get_num_seeds()
{
    return values.depth();
}

}