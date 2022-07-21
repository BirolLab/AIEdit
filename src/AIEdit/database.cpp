#include "AIEdit/database.hpp"
#include "AIEdit/data_types.hpp"
#include "AIEdit/utils.hpp"
#include <bitset>
#include <limits>

unsigned
ai_edit::PatternDatabase::distance(Signature observed, Signature from_db)
{
  unsigned distance = 0;
  for (unsigned i = 0; i < observed.get_frame_size(); i++) {
    for (unsigned j = 0; j < observed.get_num_seeds(); j++) {
      bool t = observed.get(i, j);
      bool d = from_db.get(i, j);
      if ((!t) != (!d)) {
        for (auto& entry : entries) {
          if ((t && entry.get_frame_data().get(i, j)) ||
              (!t && !entry.get_frame_data().get(i, j))) {
            ++distance;
          }
        }
      }
    }
  }
  return distance;
}

const ai_edit::PatternDatabase::Entry&
ai_edit::PatternDatabase::query(const Signature& observed,
                                unsigned& out_distance)
{
  const Entry* result = nullptr;
  unsigned min_dist = std::numeric_limits<unsigned>::max();
  for (auto& entry : entries) {
    unsigned dist = distance(observed, entry.get_frame_data());
    if (result == nullptr || dist <= min_dist) {
      result = &entry;
      min_dist = dist;
    }
  }
  out_distance = min_dist;
  return *result;
}

void
ai_edit::PatternDatabase::populate(const unsigned window_size,
                                   const unsigned int frame_size,
                                   const std::vector<SpacedSeed>& seeds)
{
  for (unsigned p = 0; p < (1U << (window_size - 1)); p++) {
    std::string pattern_str = std::bitset<64>(p).to_string() + "1";
    std::reverse(pattern_str.begin(), pattern_str.end());
    EditPattern pattern(window_size);
    for (unsigned i = 0; i < window_size; i++) {
      if (pattern_str[i] == '1') {
        pattern.set(i, EditPattern::Value::MISMATCH);
      } else {
        pattern.set(i, EditPattern::Value::CLEAN);
      }
    }
    Signature frame_data = Signature::predict(pattern, frame_size, seeds);
    entries.emplace_back(pattern, frame_data);
  }
}

nlohmann::json
ai_edit::PatternDatabase::to_json()
{
  nlohmann::json db_json;
  for (auto& entry : entries) {
    auto key = entry.get_pattern().to_string();
    auto value = entry.get_frame_data().to_string_vec();
    db_json[key] = value;
  }
  return db_json;
}
