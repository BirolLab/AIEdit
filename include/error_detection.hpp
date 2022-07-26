#ifndef AI_EDIT_ERROR_DETECTION_HPP
#define AI_EDIT_ERROR_DETECTION_HPP

#include <btllib/bloom_filter.hpp>

#include "data_types.hpp"

namespace ai_edit {

/**
 * Roll the hash function until a miss is detected.
 * @param hash_fn btllib::SeedNtHash object for generating hash values.
 * @param filter btllib::SeedBloomFilter object for interrogation.
 * @return True if the ntHash vector can advance, otherwise false.
 */
bool
find_next_miss(btllib::SeedNtHash& hash_fn,
               const btllib::SeedBloomFilter& filter);

/**
 * Update the hit/miss signature by peeking the next signature_length
 * characters in the sequence.
 * @param hash_fn btllib::SeedNtHash object for generating hash values.
 * @param btllib::SeedBloomFilter object for interrogation.
 * @param signature SignatureValue array to be updated.
 * @param signature_length Number of rows in the signature.
 */
void
update_signature(btllib::SeedNtHash& hash_fn,
                 const btllib::SeedBloomFilter& filter,
                 SignatureValue** signature,
                 const size_t signature_length);

}

#endif // AI_EDIT_ERROR_DETECTION_HPP