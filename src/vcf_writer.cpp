#include "vcf_writer.hpp"
#include "aiedit/edit.hpp"
#include "aiedit/version.hpp"

void
VCFWriter::write_headers(const std::string& assembly_path)
{
    char s[64];
    time_t t = time(0);
    strftime(s, 64, "%Y%m%d", localtime(&t));
    file << "##fileformat=VCFv4.3" << std::endl;
    file << "##fileDate=" << s << std::endl;
    file << "##source=AIEdit" << aiedit::VERSION << std::endl;
    file << "##reference=file:" << assembly_path << std::endl;
    file << "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tINTEGRATION" << std::endl;
    file.flush();
}

void
VCFWriter::write(const std::string& seq_id,
                 const std::string& seq_comment,
                 const std::vector<aiedit::Edit>& edits)
{
    for (const auto& edit : edits) {
        file << seq_id << " " << seq_comment << "\t"; // CHROM
        file << edit.position + 1 << "\t";            // POS
        file << ".\t";                                // ID
        file << edit.reference << "\t";               // REF
        file << edit.updated << "\t";                 // ALT
        file << ".\t";                                // QUAL
        file << "PASS\t";                             // FILTER
        file << ".\t";                                // INFO
        file << "GT\t";                               // FORMAT
        file << "1/1" << std::endl;                   // INTEGRATION
    }
    file.flush();
}