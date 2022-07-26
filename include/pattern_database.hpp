#ifndef AI_EDIT_PATTERN_DATABASE_HPP
#define AI_EDIT_PATTERN_DATABASE_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "data_types.hpp"

namespace ai_edit {

using PatternDatabase = std::vector<DatabaseEntry>;

struct QueryResult
{
  DatabaseEntry entry;
  unsigned distance;
};

/**
 * Build a pattern database.
 * @param seeds Vector of spaced seed patterns.
 * @param pattern_length Length of the error patterns.
 * @param signature_length Length of the signatures.
 * @return Vector of database entries (pairs of patterns and signatures).
 */
PatternDatabase
build_database(const std::vector<std::string>& seeds,
               const unsigned pattern_length,
               const unsigned signature_length);

/**
 * Find the pattern with the most similar signature in the database.
 * @param observed Querying signature.
 * @param db Pattern database.
 * @return QueryResult object containing the entry and minimum distance.
 */
QueryResult
query(SignatureValue** observed,
      const unsigned signature_length,
      const unsigned num_seeds,
      const PatternDatabase& db);

/**
 * Convert the database to a json object.
 * @param db Pattern database.
 * @param signature_length Length of the signatures in the database.
 * @param num_seeds Number of spaced seed patterns.
 * @param pattern_length Length of the patterns in the database.
 * @return JSON object representing the database.
 */
nlohmann::json
to_json(const PatternDatabase& db,
        const unsigned signature_length,
        const unsigned num_seeds,
        const unsigned pattern_length);


}

#endif // AI_EDIT_PATTERN_DATABASE_HPP