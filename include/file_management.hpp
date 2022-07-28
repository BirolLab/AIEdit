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
   * @param seq_id Sequence name.
   * @param miss_position Position of the edit pattern.
   * @param pattern_string Edit pattern string.
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