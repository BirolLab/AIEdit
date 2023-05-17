#include <btllib/bloom_filter.hpp>
#include <btllib/seq_reader.hpp>
#include <btllib/seq_writer.hpp>
#include <filesystem>
#include <fstream>
#include <fdeep/fdeep.hpp>
#include <nlohmann/json.hpp>
#include <omp.h>

#include "args.hpp"
#include "cli.hpp"
#include "timer.hpp"
#include "vcf_writer.hpp"

#include "aiedit/error_correction/mismatch_corrector.hpp"
#include "aiedit/error_detection/bloom_filter_error_detector.hpp"

size_t
get_file_size(const std::string& path)
{
    return std::filesystem::file_size(std::filesystem::path(path));
}

int
main(int argc, char** argv)
{
    ProgramArguments args;
    args.parse(argc, argv);
    std::string vcf_file_path = args.out_path / std::filesystem::path("variants.vcf");
    std::string edited_file_path = args.out_path / std::filesystem::path("edited.fa");

    omp_set_num_threads(args.num_threads);

    CommandLineInterface cli(args.verbosity);

    cli.print_logo();
    cli.print_args(args);

    cli.start_timer("Loading Bloom filter");
    btllib::SeedBloomFilter bf(args.bf_path);
    cli.stop_timer();
    cli.print_bloom_filter_information(bf);

    cli.start_timer("Loading pattern model");
    const auto model = fdeep::load_model(args.model_path);
    cli.stop_timer();
    cli.print_bloom_filter_information(bf);

    cli.start_timer("Initializing");
    aiedit::MismatchCorrector mismatch_corrector(args.pattern_length, bf, model);
    btllib::SeqReader reader(args.assembly_path, btllib::SeqReader::Flag::LONG_MODE);
    VCFWriter vcf_file(vcf_file_path, args.assembly_path);
    btllib::SeqWriter writer(edited_file_path, btllib::SeqWriter::FASTA);
    cli.stop_timer();

    cli.start_timer("Detecting and correcting errors");
    unsigned num_patterns = 0, num_mismatches = 0;
    unsigned num_hashes = bf.get_hash_num_per_seed();
    for (auto record : reader) {
        const size_t chunk_size = record.seq.size() / args.num_threads;
        std::string& seq = record.seq;
#pragma omp parallel for
        for (unsigned i = 0; i < args.num_threads; i++) {
            size_t begin = i * chunk_size;
            size_t end = i < args.num_threads - 1 ? (i + 1) * chunk_size : seq.size();
            aiedit::SequenceIterator seq_iter(seq, bf.get_seeds(), num_hashes, begin, end);
            aiedit::BloomFilterErrorDetector err_detector(seq_iter, bf);
            while (err_detector.next_error()) {
                bool fixed = mismatch_corrector.fix(seq_iter);
                if (fixed) {
                    ++num_patterns;
                } else {
                    seq_iter.next(bf.get_k() + args.pattern_length);
                }
#pragma omp critical
                {
                    cli.log_edit(record.id,
                                 fixed,
                                 seq_iter.get_position(),
                                 seq_iter.get_sequence().size());
                }
            }
        }
        vcf_file.write(record.id, record.comment, mismatch_corrector.get_edits());
        num_mismatches += mismatch_corrector.get_edits().size();
        mismatch_corrector.clear_edits();
        writer.write(record.id, record.comment, seq);
    }
    cli.stop_timer();
    cli.print_num_edits(num_patterns, num_mismatches);

    return 0;
}