#include "vcf_writer.hpp"
#include "version.hpp"

namespace aiedit {

void VCFWriter::write_headers(const std::string& assembly_path)
{
    const unsigned buffer_size = 64;
    char s[buffer_size];
    const time_t t = time(nullptr);
    // NOLINTNEXTLINE (cert-err33-c, concurrency-mt-unsafe)
    strftime(s, buffer_size, "%Y%m%d", localtime(&t));
    file << "##fileformat=VCFv4.3" << std::endl;
    file << "##fileDate=" << s << std::endl;
    file << "##source=AIEdit" << aiedit::VERSION << std::endl;
    file << "##reference=file:" << assembly_path << std::endl;
    file << "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tINTEGRATION" << std::endl;
    file.flush();
}

void VCFWriter::write(const std::string& seq_id,
                      const std::string& seq_comment,
                      const std::vector<aiedit::Edit>& edits)
{
    for (const auto& edit : edits) {
        file << seq_id << " " << seq_comment << "\t";  // CHROM
        file << edit.position + 1 << "\t";             // POS
        file << ".\t";                                 // ID
        file << edit.before << "\t";                   // REF
        file << edit.after << "\t";                    // ALT
        file << ".\t";                                 // QUAL
        file << "PASS\t";                              // FILTER
        file << ".\t";                                 // INFO
        file << "GT\t";                                // FORMAT
        file << "1/1" << std::endl;                    // INTEGRATION
    }
    file.flush();
}

}  // namespace aiedit