#pragma once

#include <cstdio>
#include <fstream>
#include <string>

#include "polisher.hpp"

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
    void write(const std::string& seq_id,
               const std::vector<std::pair<unsigned, std::string>>& ignored_patterns)
    {
#pragma omp critical
        {
            for (const auto& pattern : ignored_patterns) {
                file << seq_id << "\t" << pattern.first + 1 << "\t" << pattern.second << std::endl;
            }
        }
    }

    /**
     * Delete the created file
     */
    void delete_file() { std::remove(file_name.data()); }

  private:

    const std::string file_name;
    std::ofstream file;
};
