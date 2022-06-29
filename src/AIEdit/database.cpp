#include "AIEdit/database.hpp"
#include "AIEdit/data_types.hpp"
#include "AIEdit/utils.hpp"
#include <bitset>
#include <limits>

unsigned int
ai_edit::PatternDatabase::distance(Signature observed, Signature from_db)
{
  unsigned distance = 0;
  for (unsigned i = 0; i < observed.get_frame_size(); i++) {
    for (unsigned j = 0; j < observed.get_num_seeds(); j++) {
      bool t = observed.get(i, j);
      bool d = from_db.get(i, j);
      if ((!t) != (!d)) {
        for (auto& entry : db) {
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

const ai_edit::DatabaseEntry&
ai_edit::PatternDatabase::query(const Signature& observed,
                                unsigned& out_distance)
{
  const DatabaseEntry* result = nullptr;
  unsigned min_dist = std::numeric_limits<unsigned>::max();
  for (auto& entry : db) {
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
  for (unsigned p = 0; p < (1 << window_size); p++) {
    std::string pattern_str = std::bitset<64>(p).to_string();
    std::reverse(pattern_str.begin(), pattern_str.end());
    pattern_str = pattern_str.substr(0, window_size);
    Pattern pattern = str_to_bool_vec(pattern_str);
    Signature frame_data = Signature::predict(pattern, frame_size, seeds);
    db.emplace_back(pattern, frame_data);
  }
}

nlohmann::json
ai_edit::PatternDatabase::to_json()
{
  nlohmann::json db_json;
  for (auto& entry : db) {
    auto key = ai_edit::bool_vec_to_str(entry.get_pattern());
    auto value = entry.get_frame_data().to_string_vec();
    db_json[key] = value;
  }
  return db_json;
}

std::string
ai_edit::PatternDatabase::to_string()
{
  nlohmann::json db_json = this->to_json();
  std::string db_str = db_json.dump(4);
  return db_str;
}