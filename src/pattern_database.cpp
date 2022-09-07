#include "pattern_database.hpp"

#include <algorithm>
#include <bitset>

#include "error_detection.hpp"

namespace {

/**
 * Predict the signature for a given pattern.
 * @param pattern Input pattern.
 * @param signature_length Length of the resulting signature.
 * @param seeds Vector of spaced seed patterns.
 * @return 2D array of values representing the signature.
 */
ai_edit::Signature
predict_signature(const ai_edit::Pattern& pattern,
                  const unsigned pattern_length,
                  const unsigned signature_length,
                  const std::vector<std::string>& seeds)
{
  auto signature = ai_edit::create_signature(signature_length, seeds.size());
  for (unsigned slide = 0; slide < signature_length; slide++) {
    for (unsigned i_seed = 0; i_seed < seeds.size(); i_seed++) {
      bool miss = false;
      auto seed = seeds[i_seed];
      unsigned overlap = std::min(slide + 1, pattern_length);
      for (unsigned pos = 0; pos < overlap; pos++) {
        bool is_error = pattern[pos] == ai_edit::PatternValue::MISMATCH;
        bool is_care = seed[seed.size() - 1 - slide + pos] == '1';
        if (is_error && is_care) {
          miss = true;
        }
      }
      if (miss) {
        signature[slide][i_seed] = ai_edit::SignatureValue::MISS;
      } else {
        signature[slide][i_seed] = ai_edit::SignatureValue::HIT;
      }
    }
  }
  return signature;
}

/**
 * Compute the distance between two signatures.
 * @param observed The actual hits and misses from the Bloom filter.
 * @param from_database A postulated signature from the database.
 * @return An unsigned integer as the distance.
 */
unsigned
distance(const ai_edit::Signature& observed,
         const ai_edit::Signature& from_database,
         const unsigned signature_length,
         const unsigned num_seeds,
         const ai_edit::PatternDatabase& db)
{
  unsigned distance = 0;
  for (unsigned i = 0; i < signature_length; i++) {
    for (unsigned j = 0; j < num_seeds; j++) {
      auto t = observed[i][j];
      auto d = from_database[i][j];
      if ((!t) != (!d)) {
        for (const auto& entry : db) {
          if ((t && entry.signature[i][j]) || (!t && !entry.signature[i][j])) {
            ++distance;
          }
        }
      }
    }
  }
  return distance;
}

}

ai_edit::PatternDatabase
ai_edit::build_database(const std::vector<std::string>& seeds,
                        const unsigned pattern_length,
                        const unsigned signature_length)
{
  PatternDatabase db;
  for (unsigned p = 0; p < (1U << (pattern_length - 1)); p++) {
    std::string pattern_string = std::bitset<64>(p).to_string() + "1";
    std::reverse(pattern_string.begin(), pattern_string.end());
    PatternValue* pattern = new PatternValue[pattern_length];
    for (unsigned i = 0; i < pattern_length; i++) {
      bool is_mismatch = pattern_string[i] == '1';
      pattern[i] = (is_mismatch ? PatternValue::MISMATCH : PatternValue::CLEAN);
    }
    auto signature =
      predict_signature(pattern, pattern_length, signature_length, seeds);
    db.emplace_back(DatabaseEntry({ pattern, signature }));
  }
  return db;
}

ai_edit::QueryResult
ai_edit::query(const Signature& observed,
               const unsigned signature_length,
               const unsigned num_seeds,
               const PatternDatabase& db)
{
  const DatabaseEntry* result = nullptr;
  unsigned min_dist = std::numeric_limits<unsigned>::max();
  for (auto& entry : db) {
    unsigned dist =
      distance(observed, entry.signature, signature_length, num_seeds, db);
    if (result == nullptr || dist <= min_dist) {
      result = &entry;
      min_dist = dist;
    }
  }
  return { *result, min_dist };
}

double
ai_edit::get_database_average_signature_miss_count(
  const ai_edit::PatternDatabase& db,
  const unsigned signature_length,
  const unsigned num_seeds)
{
  unsigned sum_miss_count = 0;
  for (const auto& entry : db) {
    sum_miss_count += ai_edit::get_signature_miss_count(entry.signature,
                                                        signature_length,
                                                        num_seeds);
  }
  return (double)sum_miss_count / (double)db.size();
}