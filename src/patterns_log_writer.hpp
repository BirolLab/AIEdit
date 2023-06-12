#ifndef PATTERNS_LOG_WRITER_HPP
#define PATTERNS_LOG_WRITER_HPP

#include <fstream>
#include <string>

#include "polisher.hpp"

namespace aiedit {

class PatternsLogWriter
{
  public:

    /**
     * Construct a new pattern log writer.
     * @param path Path to the new file.
     */
    PatternsLogWriter(const std::string& path)
      : file_name(path)
      , file(path)
    {
      file << "seq\tposition\tpattern" << std::endl;
    }

    /**
     * Add a new row to the file.
     * @param seq_id Sequence name.
     * @param ignored_patterns List of ignored patterns
     */
    void write(const std::string& seq_id, const IgnoredPatternsList& ignored_patterns);

    /**
     * Delete the created file
     */
    void delete_file();

  private:

    const std::string file_name;
    std::ofstream file;
};

}  // namespace aiedit

#endif