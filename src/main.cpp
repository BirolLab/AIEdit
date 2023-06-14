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

int main(int argc, char** argv)
{
    aiedit::ProgramArguments args;
    try {
        args.parse(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return EXIT_FAILURE;
    }

    const std::string vcf_file_path = args.out_path / std::filesystem::path("variants.vcf");
    const std::string edited_file_path = args.out_path / std::filesystem::path("edited.fa");
    const std::string ignored_file_path = args.out_path / std::filesystem::path("ignored.tsv");

    omp_set_num_threads(static_cast<int>(args.num_threads));

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
#pragma omp parallel
    for (auto record : reader) {
        std::string& seq = record.seq;
        aiedit::SequenceIterator seq_iter(seq, seeds, num_hashes);
        const auto results = polisher.polish(seq_iter);
        num_mismatches += results.get_num_mismatches();
        num_insertions += results.get_num_insertions();
        num_deletions += results.get_num_deletions();
#pragma omp critical
        vcf_file.write(record.id, record.comment, results.get_edits());
#pragma omp critical
        cli.print_polisher_results(record.id, results);
        if (args.verbose) {
#pragma omp critical
            ignored_pattern_logger.write(record.id, results.get_ignored_patterns());
        }
        writer.write(record.id, record.comment, seq);
    }
    if (!args.verbose) {
        ignored_pattern_logger.delete_file();
    }
    cli.stop_timer();
    aiedit::CommandLineInterface::print_final_stats(num_mismatches, num_insertions, num_deletions);

    return 0;
}