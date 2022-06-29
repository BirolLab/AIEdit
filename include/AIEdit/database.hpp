#ifndef AIEDIT_DATABASE_HPP
#define AIEDIT_DATABASE_HPP

#include "AIEdit/data_types.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ai_edit {

class DatabaseEntry
{
private:
  Pattern pattern;
  Signature signature;

public:
  DatabaseEntry(Pattern pattern, Signature frame_data)
    : pattern(std::move(pattern))
    , signature(std::move(frame_data))
  {}

  [[nodiscard]] const Pattern& get_pattern() { return pattern; }
  [[nodiscard]] Signature get_frame_data() { return signature; }
};

class PatternDatabase
{
private:
  std::vector<DatabaseEntry> db;

  /**
   * Populate the database using the provided seeds
   */
  void populate(unsigned window_size,
                unsigned frame_size,
                const std::vector<SpacedSeed>& seeds);

  [[nodiscard]] unsigned distance(Signature observed, Signature from_db);

public:
  PatternDatabase(const unsigned window_size,
                  const unsigned frame_size,
                  const std::vector<SpacedSeed>& seeds)
  {
    populate(window_size, frame_size, seeds);
  }

  /**
   * Find the closest pattern in the database.
   * @param observed Observed hit/miss values from the data.
   * @return The most similar mismatch pattern in the database.
   */
  [[nodiscard]] const DatabaseEntry& query(const Signature& observed,
                                           unsigned& out_distance);

  /**
   * Get the JSON representation of the database.
   * @return JSON object with mismatch patterns as keys and frame data as
   * values.
   */
  [[nodiscard]] nlohmann::json to_json();

  /**
   * Get a string (JSON) representation of the database.
   * @return Database string dumped as JSON.
   */
  [[nodiscard]] std::string to_string();
};

}

#endif // AIEDIT_DATABASE_HPP
