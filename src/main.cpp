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

struct FinalStats {
    unsigned num_mismatches = 0;
    unsigned num_insertions = 0;
    unsigned num_deletions = 0;

    void merge(const aiedit::PolishingResults& results)
    {
        num_mismatches += results.get_num_mismatches();
        num_insertions += results.get_num_insertions();
        num_deletions += results.get_num_deletions();
    }
};

inline std::string join_strings(const std::vector<std::string>& strings, size_t size)
{
    std::string result;
    result.reserve(size);
    for (const auto& s : strings) {
        result.append(s);
    }
    return result;
}

inline FinalStats polish_contigs(aiedit::Polisher& polisher,
                                 btllib::SeqReader& seq_reader,
                                 const std::vector<std::string>& seeds,
                                 unsigned num_hashes,
                                 aiedit::CommandLineInterface& cli,
                                 btllib::SeqWriter& seq_writer,
                                 aiedit::VCFWriter& vcf_writer,
                                 aiedit::PatternsLogWriter& ignored_patterns_writer)
{
    FinalStats stats;
#pragma omp parallel
    for (auto record : seq_reader) {
        aiedit::SequenceIterator seq_iter(record.seq, seeds, num_hashes);
        auto results = polisher.polish(seq_iter);
        stats.merge(results);
        vcf_writer.write(record.id, record.comment, results.get_edits());
        cli.print_polisher_results(record.id, record.seq.size(), 0, results);
        ignored_patterns_writer.write(record.id, results.get_ignored_patterns());
        seq_writer.write(record.id, record.comment, record.seq);
    }
    return stats;
}

inline FinalStats polish_assembly(aiedit::Polisher& polisher,
                                  btllib::SeqReader& seq_reader,
                                  const std::vector<std::string>& seeds,
                                  unsigned num_hashes,
                                  unsigned num_threads,
                                  aiedit::CommandLineInterface& cli,
                                  btllib::SeqWriter& seq_writer,
                                  aiedit::VCFWriter& vcf_writer,
                                  aiedit::PatternsLogWriter& ignored_patterns_writer)
{
    FinalStats stats;
    for (auto record : seq_reader) {
        bool too_short = record.seq.size() < seeds[0].size() * num_threads;
        unsigned num_chunks = too_short ? 1 : num_threads;
        const unsigned chunk_size = record.seq.size() / num_chunks;
        std::vector<std::string> polished;
        polished.resize(num_chunks);
#pragma omp parallel for num_threads(num_chunks)
        for (unsigned i = 0; i < num_chunks; i++) {
            const unsigned begin = i * chunk_size;
            const unsigned end = i < num_chunks - 1 ? (i + 1) * chunk_size : record.seq.size();
            const unsigned length = end - begin + (i < num_chunks - 1 ? seeds[0].size() : 0);
            std::string chunk = record.seq.substr(begin, length);
            aiedit::SequenceIterator seq_iter(chunk, seeds, num_hashes);
            auto results = polisher.polish(seq_iter);
            if (i > 0) {
                size_t chunk_begin = seeds[0].size();
                size_t chunk_end = chunk.size() - seeds[0].size();
                polished[i] = std::move(chunk.substr(chunk_begin, chunk_end));
            } else {
                polished[i] = std::move(chunk);
            }
            stats.merge(results);
            vcf_writer.write(record.id, record.comment, results.get_edits());
            cli.print_polisher_results(record.id, record.seq.size(), i, results);
            ignored_patterns_writer.write(record.id, results.get_ignored_patterns());
        }
        seq_writer.write(record.id, record.comment, join_strings(polished, record.seq.size()));
    }
    return stats;
}

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
    aiedit::PatternsLogWriter ignored_patterns_writer(prefix + "-aiedit-ignored.tsv");
    btllib::SeqWriter seq_writer(prefix + "-aiedit-polished.fa", btllib::SeqWriter::FASTA);
    btllib::SeqReader seq_reader(args.in_path, btllib::SeqReader::Flag::LONG_MODE);

    cli.start_timer("Detecting and correcting errors");
    omp_set_num_threads(args.num_threads);
    aiedit::Polisher polisher(model_json["pattern_length"], bf, model);
    FinalStats stats;
    if (args.contig_mode) {
        stats = polish_contigs(polisher,
                               seq_reader,
                               seeds,
                               bf.get_hash_num(),
                               cli,
                               seq_writer,
                               vcf_writer,
                               ignored_patterns_writer);
    } else {
        stats = polish_assembly(polisher,
                                seq_reader,
                                seeds,
                                bf.get_hash_num(),
                                args.num_threads,
                                cli,
                                seq_writer,
                                vcf_writer,
                                ignored_patterns_writer);
    }
    cli.stop_timer();
    aiedit::CommandLineInterface::print_final_stats(stats.num_mismatches,
                                                    stats.num_insertions,
                                                    stats.num_deletions);

    return 0;
}