#include "patterns_log_writer.hpp"

#include <cstdio>

namespace aiedit {

void PatternsLogWriter::write(const std::string& seq_id,
                              const IgnoredPatternsList& ignored_patterns)
{
    for (const auto& pattern : ignored_patterns) {
        file << seq_id << "\t" << pattern.first << "\t" << pattern.second << std::endl;
    }
}

void PatternsLogWriter::delete_file() { std::remove(file_name.data()); }

}  // namespace aiedit