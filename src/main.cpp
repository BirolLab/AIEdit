#include <btllib/bloom_filter.hpp>
#include <btllib/seq_reader.hpp>
#include <btllib/seq_writer.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <omp.h>

#include "args.hpp"
#include "cli.hpp"
#include "timer.hpp"
#include "vcf_writer.hpp"

#include "aiedit/error_detection/bloom_filter_error_detector.hpp"
#include "aiedit/error_detection/error_detector.hpp"

#include "aiedit/pattern_detection/pattern_database.hpp"
#include "aiedit/pattern_detection/pattern_detector.hpp"

#include "aiedit/error_correction/bloom_filter_mismatch_corrector.hpp"
#include "aiedit/error_correction/error_corrector.hpp"

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

    omp_set_num_threads(args.num_threads);

    CommandLineInterface cli(args.verbosity);

    cli.print_logo();
    cli.print_args(args);

    cli.start_timer("Loading Bloom filter");
    btllib::SeedBloomFilter bf(args.bf_path);
    cli.stop_timer();
    cli.print_bloom_filter_information(bf, args.bf_path);

    aiedit::PatternDetector* pattern_detector = nullptr;
    cli.start_timer("Populating pattern database");
    pattern_detector = new aiedit::PatternDatabase(args.pattern_length, bf.get_seeds());
    std::ofstream db_file(args.out_path / std::filesystem::path("db.json"));
    auto db_json = dynamic_cast<aiedit::PatternDatabase*>(pattern_detector)->to_json();
    db_file << db_json.dump(4);
    db_file.flush();
    cli.stop_timer();

    btllib::SeqReader reader(args.assembly_path, btllib::SeqReader::Flag::LONG_MODE);

    std::string vcf_file_path = args.out_path / std::filesystem::path("variants.vcf");
    VCFWriter vcf_file(vcf_file_path, args.assembly_path);

    std::string edited_file_path = args.out_path / std::filesystem::path("edited.fa");
    btllib::SeqWriter writer(edited_file_path, btllib::SeqWriter::FASTA);

    cli.start_timer("Detecting and correcting errors");
    unsigned num_patterns = 0, num_mismatches = 0;
    unsigned num_hashes = bf.get_hash_num_per_seed();
    unsigned kmer_length = bf.get_seeds()[0].size();
    for (auto record : reader) {
        const size_t chunk_size = record.seq.size() / args.num_threads;
        std::string& seq = record.seq;
#pragma omp parallel for
        for (unsigned i = 0; i < args.num_threads; i++) {
            size_t begin = i * chunk_size;
            size_t end = i < args.num_threads - 1 ? (i + 1) * chunk_size : seq.size();
            aiedit::SequenceIterator seq_iter(seq, bf.get_seeds(), num_hashes, begin, end);
            aiedit::BloomFilterErrorDetector err_detector(seq_iter, bf);
            aiedit::BloomFilterMismatchCorrector err_corrector(seq_iter, bf);
            while (err_detector.next_error()) {
                aiedit::Signature signature(seq_iter.peek_hashes(kmer_length).data(), bf);
                auto& pattern = pattern_detector->get_pattern(signature);
                bool fixed = err_corrector.fix(pattern);
                if (fixed) {
                    ++num_patterns;
                } else {
                    seq_iter.next(bf.get_k() + args.pattern_length);
                }
                cli.log_edit(record.id,
                             seq_iter.get_position(),
                             seq_iter.get_sequence().size(),
                             pattern.to_string(),
                             fixed);
            }
#pragma omp critical
            {
                vcf_file.write(record.id, record.comment, err_corrector.get_edits());
                num_mismatches += err_corrector.get_edits().size();
            }
        }
        writer.write(record.id, record.comment, seq);
    }
    cli.stop_timer();
    cli.print_num_edits(num_patterns, num_mismatches);

    return 0;
}