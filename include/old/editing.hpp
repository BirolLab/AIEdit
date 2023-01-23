#ifndef AI_EDIT_EDITING_HPP
#define AI_EDIT_EDITING_HPP

#include <btllib/bloom_filter.hpp>
#include <vector>

#include "data_types.hpp"
#include "nthash/nthash.hpp"

namespace ai_edit {

struct Edit
{
  size_t position;
  char content;
};

/**
 * Get the edits that will polish the sequence. This function will NOT edit the
 * sequence contents but requires the sequence string to be editable.
 * @param seq Sequence to be polished.
 * @param position Position of the edit region.
 * @param pattern Edit pattern detected by the algorithm.
 * @param pattern_length Length of the edit pattern.
 * @param bloom_filter Bloom filter for interrogation.
 * @param hash_function Spaced seed hash function (SeedNtHash object).
 * @return Vector of edit positions in the sequence and replacement content.
 */
std::vector<Edit>
get_edits(std::string& seq,
          const size_t position,
          const Pattern& pattern,
          const unsigned pattern_length,
          const btllib::SeedBloomFilter& bloom_filter,
          nthash::SeedNtHash& hash_function,
          const unsigned signature_length);

/**
 * Apply edits to the sequence in-place.
 * @param seq Sequence to be edited.
 * @param edits List of edit positions and contents to be replaced in the
 * sequence.
 */
void
apply_edits(std::string& seq,
            nthash::SeedNtHash& hash_function,
            const std::vector<Edit>& edits);

}

#endif // AI_EDIT_EDITING_HPP