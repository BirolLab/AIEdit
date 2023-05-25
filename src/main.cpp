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
    btllib::SeqWriter writer(edited_file_path, btllib::SeqWriter::FASTA);

    cli.start_timer("Detecting and correcting errors");
    aiedit::Polisher polisher(pattern_length, bf, model);
    unsigned num_mismatches = 0, num_insertions = 0, num_deletions = 0;
    for (auto record : reader) {
        std::string& seq = record.seq;
        const unsigned chunk_size = seq.size() / args.num_threads;
#pragma omp parallel for
        for (unsigned i = 0; i < args.num_threads; i++) {
            const unsigned begin = i * chunk_size;
            const unsigned end = i < args.num_threads - 1 ? (i + 1) * chunk_size : seq.size();
            aiedit::SequenceIterator seq_iter(seq, seeds, num_hashes, begin, end);
            const auto results = polisher.polish(seq_iter);
            num_mismatches += results.get_num_mismatches();
            num_insertions += results.get_num_insertions();
            num_deletions += results.get_num_deletions();
#pragma omp critical
            vcf_file.write(record.id, record.comment, results.get_edits());
#pragma omp critical
            cli.print_polisher_results(record.id, results);
        }
        writer.write(record.id, record.comment, seq);
    }
    cli.stop_timer();
    aiedit::CommandLineInterface::print_final_stats(num_mismatches, num_insertions, num_deletions);

    return 0;
}