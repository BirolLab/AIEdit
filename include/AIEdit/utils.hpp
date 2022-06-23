#ifndef AIEDIT_UTILS_HPP
#define AIEDIT_UTILS_HPP

#include <string>
#include <vector>

namespace ai_edit {

/**
 * Convert a string containing zeros and ones to a vector of booleans.
 * True values represent ones.
 * @param str Input string.
 * @return Resulting vector of boolean values.
 */
std::vector<bool>
str_to_bool_vec(const std::string& str);

/**
 * Convert a bool string to a binary string.
 * Ones/zeros represent true/false values.
 * @param vec Input vector.
 * @return Resulting binary string.
 */
std::string
bool_vec_to_str(const std::vector<bool>& vec);

/**
 * Check if file exists.
 * @param path Path to file.
 * @return True if the file exists, otherwise false.
 */
bool
file_exists(const std::string& path);

}

#endif // AIEDIT_UTILS_HPP
