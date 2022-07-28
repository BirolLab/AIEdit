#include "file_management.hpp"

#include <nlohmann/json.hpp>

#include "pattern_database.hpp"

void
ai_edit::write_database_file(const ai_edit::PatternDatabase& database,
                             const unsigned signature_length,
                             const unsigned num_seeds,
                             const unsigned pattern_length,
                             const std::string& path)
{
  nlohmann::json db_json;
  for (auto& entry : database) {
    auto key = ai_edit::pattern_to_string(entry.pattern, pattern_length);
    auto value = ai_edit::signature_to_string_vec(entry.signature,
                                                  signature_length,
                                                  num_seeds);
    db_json[key] = value;
  }
  std::ofstream db_file(path);
  db_file << db_json.dump(4);
  db_file.flush();
}

void
ai_edit::EditsFile::write(const std::string& seq,
                          const std::string& seq_id,
                          const size_t miss_position,
                          const ai_edit::Pattern& pattern,
                          const unsigned pattern_length,
                          const std::vector<ai_edit::Edit>& edits,
                          const unsigned distance)
{
  file << "\"" << seq_id << "\"\t" << miss_position + 1 << "\t";
  std::string before = std::string(pattern_length, '-');
  std::string after = std::string(pattern_length, '-');
  for (const auto& edit : edits) {
    before[edit.position - miss_position] = seq[edit.position];
    after[edit.position - miss_position] = edit.content;
  }
  file << before << "\t" << after << "\t" << distance << std::endl;
}