#pragma once

#include <fstream>
#include <vector>

#include "edit.hpp"

class VCFWriter
{
  public:

    /**
     * Construct a new VCF file writer.
     * @param path Path to the new VCF file.
     * @param assembly_path Path to the input assembly file.
     */
    VCFWriter(const std::string& path, const std::string& assembly_path, const std::string& version)
      : file(path)
    {
        write_headers(assembly_path, version);
    }

    /**
     * Add a new row to the file.
     * @param seq_id Sequence name.
     * @param seq_comment Sequence comment in header.
     * @param edits List of edits
     */
    void
    write(const std::string& seq_id, const std::string& seq_comment, const std::vector<Edit>& edits)
    {
#pragma omp critical
        {
            for (const auto& edit : edits) {
                file << seq_id << (seq_comment != "" ? " " : "") << seq_comment << "\t";  // CHROM
                file << edit.get_position() + 1 << "\t";                                  // POS
                file << ".\t";                                                            // ID
                file << edit.get_before() << "\t";                                        // REF
                file << edit.get_after() << "\t";                                         // ALT
                file << ".\t";                                                            // QUAL
                file << "PASS\t";                                                         // FILTER
                file << ".\t";                                                            // INFO
                file << "GT\t";                                                           // FORMAT
                file << "1/1" << std::endl;  // INTEGRATION
            }
        }
    }

  private:

    std::ofstream file;

    /**
     * Write the VCF file's headers.
     */
    void write_headers(const std::string& assembly_path, const std::string& version)
    {
        const unsigned buffer_size = 64;
        char s[buffer_size];
        const time_t t = time(nullptr);
        // NOLINTNEXTLINE (cert-err33-c, concurrency-mt-unsafe)
        strftime(s, buffer_size, "%Y%m%d", localtime(&t));
        file << "##fileformat=VCFv4.3" << std::endl;
        file << "##fileDate=" << s << std::endl;
        file << "##source=AIEdit" << version << std::endl;
        file << "##reference=file:" << assembly_path << std::endl;
        file << "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tINTEGRATION" << std::endl;
    }
};
