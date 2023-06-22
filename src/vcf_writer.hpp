#ifndef VCF_WRITER_HPP
#define VCF_WRITER_HPP

#include <fstream>
#include <vector>

#include "edit.hpp"

namespace aiedit {

class VCFWriter
{
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
     * @param seq_id Sequence name.
     * @param seq_comment Sequence comment in header.
     * @param edits List of edits
     */
    void write(const std::string& seq_id,
               const std::string& seq_comment,
               const std::vector<aiedit::Edit>& edits);

  private:

    std::ofstream file;

    /**
     * Write the VCF file's headers.
     */
    void write_headers(const std::string& assembly_path);
};

}  // namespace aiedit

#endif  // VCF_WRITER_HPP