#include "file_management.hpp"

#include <ctime>
#include <filesystem>
#include <nlohmann/json.hpp>

#include "pattern_database.hpp"
#include "user_interface.hpp"

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
ai_edit::VCFWriter::write_headers(const std::string& assembly_path)
{
  char s[64];
  time_t t = time(0);
  strftime(s, 64, "%Y%m%d", localtime(&t));
  file << "##fileformat=VCFv4.3" << std::endl;
  file << "##fileDate=" << s << std::endl;
  file << "##source=AIEdit" << VERSION << std::endl;
  file << "##reference=file:" << assembly_path << std::endl;
  file << "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tINTEGRATION"
       << std::endl;
}

void
ai_edit::VCFWriter::write(const std::string& seq,
                          const std::string& seq_id,
                          const std::string& seq_comment,
                          const std::vector<ai_edit::Edit>& edits)
{
  for (const auto& edit : edits) {
    file << seq_id << " " << seq_comment << "\t"; // CHROM
    file << edit.position + 1 << "\t";            // POS
    file << ".\t";                                // ID
    file << seq[edit.position] << "\t";           // REF
    file << edit.content << "\t";                 // ALT
    file << ".\t";                                // QUAL
    file << "PASS\t";                             // FILTER
    file << ".\t";                                // INFO
    file << "GT\t";                               // FORMAT
    file << "1/1" << std::endl;                   // INTEGRATION
  }
}

void
ai_edit::EditsFile::write(const std::string& seq,
                          const std::string& seq_id,
                          const std::string& seq_comment,
                          const size_t miss_position,
                          const ai_edit::Pattern& pattern,
                          const unsigned pattern_length,
                          const std::vector<ai_edit::Edit>& edits,
                          const unsigned distance)
{
  file << "\"" << seq_id << " " << seq_comment << "\"\t" << miss_position + 1
       << "\t";
  std::string before = std::string(pattern_length, '-');
  std::string after = std::string(pattern_length, '-');
  for (const auto& edit : edits) {
    before[edit.position - miss_position] = seq[edit.position];
    after[edit.position - miss_position] = edit.content;
  }
  file << before << "\t" << after << "\t" << distance << std::endl;
}

size_t
ai_edit::get_file_size(const std::string& path)
{
  return std::filesystem::file_size(std::filesystem::path(path));
}