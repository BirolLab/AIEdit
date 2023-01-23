#include "sequence_iterator.hpp"

namespace aiedit {

void
SequenceIterator::next()
{
  can_roll = nthash->roll();
}

bool
SequenceIterator::has_next()
{
  return can_roll;
}

size_t
SequenceIterator::get_position()
{
  return nthash->get_pos();
}

std::vector<const uint64_t*>
SequenceIterator::get_hashes()
{
  std::vector<const uint64_t*> hashes;
  for (unsigned i = 0; i < seeds.size(); i++) {
    uint64_t* seed_hashes = new uint64_t[nthash->get_hash_num_per_seed()];
    std::copy(nthash->hashes(),
              nthash->hashes() + nthash->get_hash_num_per_seed(),
              seed_hashes);
    hashes.push_back(seed_hashes);
  }
  return hashes;
}

void
SequenceIterator::update(size_t position, char value)
{}

void
SequenceIterator::insert(size_t position, char value)
{}

void
SequenceIterator::remove(size_t position)
{}

};