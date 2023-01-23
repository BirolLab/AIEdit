#include "signature.hpp"

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

};