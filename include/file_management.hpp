#ifndef AI_EDIT_FILE_MANAGEMENT
#define AI_EDIT_FILE_MANAGEMENT

#include <fstream>

#include "editing.hpp"
#include "pattern_database.hpp"

namespace ai_edit {

/**
 * File management for logging detected edit patterns.
 */
class EditsFile
{
private:
  std::ofstream file;

public:
  /**
   * Create a new edits file.
   * @param path Path to edits file.
   */
  EditsFile(const std::string& path)
    : file(path)
  {}

  /**
   * Add a new row to the file.
   * @param seq Sequence contents.
   * @param seq_id Sequence name.
   * @param miss_position Position of the edit pattern.
   * @param pattern Edit pattern.
   * @param pattern_length Edit pattern length.
   * @param edits List of edits
   * @param distance Distance between the observed signature and the signature
   * from the pattern database.
   */
  void write(const std::string& seq,
             const std::string& seq_id,
             const size_t miss_position,
             const ai_edit::Pattern& pattern,
             const unsigned pattern_length,
             const std::vector<ai_edit::Edit>& edits,
             const unsigned distance);
};

class VCFWriter
{
private:
  std::ofstream file;

  /**
   * Write the VCF file's headers.
   */
  void write_headers(const std::string& assembly_path);

public:
  /**
   * Construct a new VCF file writer.
   * @param path Path to the new VCF file.
   * @param assembly_path Path to the input assembly file.
   */
  VCFWriter(const std::string& path, const std::string& assembly_path)
    : file(path)
  {
    write_headers(assembly_path);
  }

  /**
   * Add a new row to the file.
   * @param seq Sequence contents.
   * @param seq_id Sequence name.
   * @param edits List of edits
   */
  void write(const std::string& seq,
             const std::string& seq_id,
             const std::vector<ai_edit::Edit>& edits);
};

/**
 * Write pattern database to a json file.
 * @param database Populated pattern database.
 * @param signature_length Length of signatures.
 * @param num_seeds Number of spaced seed patterns.
 * @param pattern_length Edit pattern length.
 * @param path Database's JSON file path.
 */
void
write_database_file(const ai_edit::PatternDatabase& database,
                    const unsigned signature_length,
                    const unsigned num_seeds,
                    const unsigned pattern_length,
                    const std::string& path);

}

#endif // AI_EDIT_FILE_MANAGEMENT