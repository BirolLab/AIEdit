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
        std::cerr << args.get_help_message();
        return EXIT_FAILURE;
    }

    aiedit::CommandLineInterface cli(args.verbose);

    cli.print_logo();
    cli.print_args(args);
    omp_set_num_threads(args.num_threads);

    cli.start_timer("Loading counting Bloom filter");
    const btllib::CountingBloomFilter8 bf(args.bf_path);
    cli.stop_timer();
    cli.print_bloom_filter_info(bf);

    cli.start_timer("Loading pattern detector model");
    const auto model = fdeep::load_model(args.model_path, false, fdeep::dev_null_logger);
    const auto model_json = nlohmann::json::parse(std::ifstream(args.model_path));
    const std::vector<std::string> seeds = model_json["seeds"];
    cli.stop_timer();
    cli.print_model_info(model_json);

    const std::string prefix = args.out_path / std::filesystem::path(args.in_path).stem();
    aiedit::VCFWriter vcf_writer(prefix + "-aiedit-variants.vcf", args.in_path);
    aiedit::PatternsLogWriter ignored_writer(prefix + "-aiedit-ignored.tsv");
    btllib::SeqWriter seq_writer(prefix + "-aiedit-polished.fa", btllib::SeqWriter::FASTA);
    btllib::SeqReader seq_reader(args.in_path, btllib::SeqReader::Flag::LONG_MODE);

    cli.start_timer("Detecting and correcting errors");
    aiedit::Polisher polisher(model_json["pattern_length"], bf, model);
    unsigned num_mismatches = 0;
    unsigned num_insertions = 0;
    unsigned num_deletions = 0;
    if (args.contig_mode) {
#pragma omp parallel
        for (auto record : seq_reader) {
            aiedit::SequenceIterator seq_iter(record.seq, seeds, bf.get_hash_num());
            auto results = polisher.polish(seq_iter);
            num_mismatches += results.get_num_mismatches();
            num_insertions += results.get_num_insertions();
            num_deletions += results.get_num_deletions();
            vcf_writer.write(record.id, record.comment, results.get_edits());
            ignored_writer.write(record.id, results.get_ignored_patterns());
            cli.print_polisher_results(record.id, record.seq.size(), 0, results);
        }
    } else {
        for (auto record : seq_reader) {
            bool too_short = record.seq.size() < seeds[0].size() * args.num_threads;
            unsigned num_chunks = too_short ? 1 : args.num_threads;
            const unsigned chunk_size = record.seq.size() / num_chunks;
#pragma omp parallel for num_threads(num_chunks)
            for (unsigned i = 0; i < num_chunks; i++) {
                const unsigned begin = i * chunk_size;
                const unsigned end = i < num_chunks - 1 ? (i + 1) * chunk_size : record.seq.size();
                aiedit::SequenceIterator seq_iter(record.seq, seeds, bf.get_hash_num(), begin, end);
                auto results = polisher.polish(seq_iter);
                num_mismatches += results.get_num_mismatches();
                num_insertions += results.get_num_insertions();
                num_deletions += results.get_num_deletions();
                vcf_writer.write(record.id, record.comment, results.get_edits());
                ignored_writer.write(record.id, results.get_ignored_patterns());
                cli.print_polisher_results(record.id, record.seq.size(), i, results);
            }
        }
    }
    cli.stop_timer();
    aiedit::CommandLineInterface::print_final_stats(num_mismatches, num_insertions, num_deletions);

    return 0;
}