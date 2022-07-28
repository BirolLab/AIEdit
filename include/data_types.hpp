#ifndef AI_EDIT_DATA_TYPES_HPP
#define AI_EDIT_DATA_TYPES_HPP

#include <stddef.h>
#include <string>
#include <vector>

namespace ai_edit {

enum SignatureValue
{
  HIT,
  MISS
};

enum PatternValue
{
  CLEAN,
  MISMATCH
};

struct DatabaseEntry
{
  PatternValue* pattern;
  SignatureValue** signature;
};

/**
 * Create a new Signature object.
 * @param length Length of the signature (number of rows).
 * @param num_seeds Number of spaced seed patterns (number of columns).
 * @return An empty signature filled with SignatureValue::HIT values
 */
SignatureValue**
create_signature(const size_t length, const unsigned num_seeds);

/**
 * Convert a signature to a vector of strings.
 * @param signature Input signature.
 * @return Vector of strings, each a row of the given signature.
 */
std::vector<std::string>
to_string_vec(SignatureValue** signature,
              const unsigned signature_length,
              const unsigned num_seeds);

/**
 * Convert an error pattern to a string.
 * @param pattern Input pattern.
 * @return String representation of the pattern.
 */
std::string
to_string(PatternValue* pattern, const unsigned pattern_length);

/**
 * Get the edit positions of a pattern.
 * @param pattern Pattern values.
 * @param pattern_length Length of the pattern array.
 * @return Pattern's edit positions relative to the start of the pattern.
 */
std::vector<size_t>
get_edit_positions(PatternValue* pattern, const unsigned pattern_length);

}

#endif // AI_EDIT_DATA_TYPES_HPP