#include <btllib/counting_bloom_filter.hpp>
#include <btllib/seq_reader.hpp>
#include <btllib/seq_writer.hpp>
#include <fdeep/fdeep.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <omp.h>
#include <set>

#include "args.hpp"
#include "cli.hpp"
#include "patterns_log_writer.hpp"
#include "polisher.hpp"
#include "sequence_iterator.hpp"
#include "timer.hpp"
#include "vcf_writer.hpp"

inline std::string join_strings(const std::vector<std::string>& strings, size_t size)
{
    std::string result;
    result.reserve(size);
    for (const auto& s : strings) {
        result.append(s);
    }
    return result;
}

int main(int argc, char** argv)
{
    aiedit::ProgramArguments args;
    try {
        args.parse(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return EXIT_FAILURE;
    }

    const std::string prefix = args.out_path / std::filesystem::path(args.assembly_path).stem();
    const std::string vcf_file_path = prefix + "-aiedit-variants.vcf";
    const std::string edited_file_path = prefix + "-aiedit-polished.fa";
    const std::string ignored_file_path = prefix + "-aiedit-ignored.tsv";

    aiedit::CommandLineInterface cli(args.verbose);

    cli.print_logo();
    cli.print_args(args);

    cli.start_timer("Loading counting Bloom filter");
    const btllib::CountingBloomFilter8 bf(args.bf_path);
    const unsigned num_hashes = bf.get_hash_num();
    cli.stop_timer();
    cli.print_bloom_filter_information(bf);

    cli.start_timer("Loading pattern detector model");
    const auto model = fdeep::load_model(args.model_path, false, fdeep::dev_null_logger);
    const auto model_json = nlohmann::json::parse(std::ifstream(args.model_path));
    const std::vector<std::string> seeds = model_json["seeds"];
    const unsigned seed_length = seeds[0].size();
    const unsigned pattern_length = model_json["pattern_length"];
    cli.stop_timer();
    cli.print_model_information(model_json);

    btllib::SeqReader reader(args.assembly_path, btllib::SeqReader::Flag::LONG_MODE);
    aiedit::VCFWriter vcf_file(vcf_file_path, args.assembly_path);
    aiedit::PatternsLogWriter ignored_pattern_logger(ignored_file_path);
    btllib::SeqWriter writer(edited_file_path, btllib::SeqWriter::FASTA);

    cli.start_timer("Detecting and correcting errors");
    aiedit::Polisher polisher(pattern_length, bf, model);
    unsigned num_mismatches = 0, num_insertions = 0, num_deletions = 0;
    const unsigned num_parallel_records = args.contig_mode ? args.num_threads : 1;
#pragma omp parallel num_threads(num_parallel_records)
    for (auto record : reader) {
        std::string seq = record.seq;
        unsigned num_chunks, chunk_size;
        if (seq.size() < seed_length) {
            continue;
        } else if (seq.size() < seed_length * args.num_threads || args.contig_mode) {
            num_chunks = 1;
            chunk_size = seq.size();
        } else {
            num_chunks = args.num_threads;
            chunk_size = seq.size() / args.num_threads;
        }
        std::vector<std::string> chunk_seqs;
        chunk_seqs.resize(num_chunks);
#pragma omp parallel for num_threads(num_chunks)
        for (unsigned i = 0; i < num_chunks; i++) {
            const unsigned begin = i * chunk_size;
            const unsigned end = i < num_chunks - 1 ? (i + 1) * chunk_size : seq.size();
            std::string chunk_seq = seq.substr(begin, end - begin + seed_length);
            aiedit::SequenceIterator seq_iter(chunk_seq, seeds, num_hashes);
            const auto results = polisher.polish(seq_iter);
            if (i > 0) {
                chunk_seq = chunk_seq.substr(seed_length, chunk_seq.size() - seed_length);
            }
            chunk_seqs[i] = chunk_seq;
            num_mismatches += results.get_num_mismatches();
            num_insertions += results.get_num_insertions();
            num_deletions += results.get_num_deletions();
#pragma omp critical
            vcf_file.write(record.id, record.comment, results.get_edits());
#pragma omp critical
            cli.print_polisher_results(record.id, chunk_seq.size(), i, results);
            if (args.verbose) {
#pragma omp critical
                ignored_pattern_logger.write(record.id, results.get_ignored_patterns());
            }
        }
        writer.write(record.id, record.comment, join_strings(chunk_seqs, seq.size()));
    }
    if (!args.verbose) {
        ignored_pattern_logger.delete_file();
    }
    cli.stop_timer();
    aiedit::CommandLineInterface::print_final_stats(num_mismatches, num_insertions, num_deletions);

    return 0;
}