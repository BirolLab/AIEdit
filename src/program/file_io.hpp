#pragma once

#include <algorithm>
#include <fstream>
#include <string>

#include "core/aiedit.hpp"
#include "core/edit.hpp"

class EditsListWriter
{

  public:

    EditsListWriter(const std::string& path, char delim = '\t')
      : os(path)
      , delim(delim)
    {
        os << "seq_id" << delim;
        os << "position" << delim;
        os << "type" << delim;
        os << "before" << delim;
        os << "after" << std::endl;
    }

    void write(const std::string& seq_id, const aiedit::Edit& edit)
    {
        os << seq_id << delim;
        os << edit.pos << delim;
        os << static_cast<char>(edit.get_type()) << delim;
        os << edit.before << delim;
        os << edit.after << std::endl;
    }

  private:

    std::ofstream os;
    const char delim;
};

class IgnoredPatternsWriter
{
  public:

    IgnoredPatternsWriter(const std::string& path, char delim = '\t')
      : os(path)
      , delim(delim)
    {
        os << "seq_id" << delim;
        os << "position" << delim;
        os << "pattern" << std::endl;
    }

    void write(const std::string& seq_id, const aiedit::IgnoredPattern& pattern)
    {
        std::string pattern_str(pattern.model_output.size(), ' ');
        std::transform(pattern.model_output.begin(),
                       pattern.model_output.end(),
                       pattern_str.begin(),
                       [](aiedit::Edit::Type t) { return static_cast<char>(t); });
        os << seq_id << delim;
        os << pattern.pos << delim;
        os << pattern_str << std::endl;
    }

  private:

    std::ofstream os;
    const char delim;
};