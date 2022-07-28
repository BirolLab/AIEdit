#ifndef AI_EDIT_ERROR_DETECTION_HPP
#define AI_EDIT_ERROR_DETECTION_HPP

#include <btllib/bloom_filter.hpp>

#include "data_types.hpp"
#include "nthash/nthash.hpp"

namespace ai_edit {

/**
 * Roll the hash function until a miss is detected.
 * @param hash_function btllib::SeedNtHash object for generating hash values.
 * @param filter btllib::SeedBloomFilter object for interrogation.
 * @return True if the ntHash vector can advance, otherwise false.
 */
bool
roll_to_next_miss(nthash::SeedNtHash& hash_function,
                  const btllib::SeedBloomFilter& filter);

/**
 * Update the hit/miss signature by peeking the next signature_length
 * characters in the sequence.
 * @param hash_fn btllib::SeedNtHash object for generating hash values. The
 * position of the rolling hash should remain the same after calling
 * this function.
 * @param btllib::SeedBloomFilter object for interrogation.
 * @param signature SignatureValue array to be updated.
 * @param signature_length Number of rows in the signature.
 * @return True if the signature contains any misses, otherwise false.
 */
bool
update_signature(nthash::SeedNtHash& hash_fn,
                 const btllib::SeedBloomFilter& filter,
                 Signature& signature,
                 const size_t signature_length);

}

#endif // AI_EDIT_ERROR_DETECTION_HPP