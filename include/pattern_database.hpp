#ifndef AI_EDIT_PATTERN_DATABASE_HPP
#define AI_EDIT_PATTERN_DATABASE_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "data_types.hpp"

namespace ai_edit {

struct DatabaseEntry
{
  Pattern pattern;
  Signature signature;
};

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
query(const Signature& observed,
      const unsigned signature_length,
      const unsigned num_seeds,
      const PatternDatabase& db);

}

#endif // AI_EDIT_PATTERN_DATABASE_HPP