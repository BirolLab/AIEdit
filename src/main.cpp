#include <btllib/bloom_filter.hpp>
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
#include "timer.hpp"
#include "vcf_writer.hpp"

#include "error_detector.hpp"
#include "pattern_detector.hpp"
#include "mismatch_corrector.hpp"

inline bool verify_seeds(const std::vector<std::string>& bf_seeds,
                         const std::vector<std::string>& model_seeds)
{
    const std::set<std::string> bf_set(bf_seeds.begin(), bf_seeds.end());
    const std::set<std::string> model_set(model_seeds.begin(), model_seeds.end());
    return bf_set == model_set;
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

    const std::string vcf_file_path = args.out_path / std::filesystem::path("variants.vcf");
    const std::string edited_file_path = args.out_path / std::filesystem::path("edited.fa");

    omp_set_num_threads(static_cast<int>(args.num_threads));

    aiedit::CommandLineInterface cli(args.verbosity);

    cli.print_logo();
    cli.print_args(args);

    cli.start_timer("Loading Bloom filter");
    const btllib::SeedBloomFilter bf(args.bf_path);
    cli.stop_timer();
    cli.print_bloom_filter_information(bf);

    cli.start_timer("Loading pattern detector model");
    const auto model = fdeep::load_model(args.model_path, false, nullptr);
    const auto model_json = nlohmann::json::parse(std::ifstream(args.model_path));
    const unsigned pattern_length = model_json["pattern_length"];
    cli.stop_timer();
    cli.print_model_information(model_json);

    const bool same_seeds = verify_seeds(bf.get_seeds(), model_json["seeds"]);
    btllib::check_error(!same_seeds, "Bloom filter and model spaced seed set are not the same");

    aiedit::MismatchCorrector mismatch_corrector(pattern_length, bf);
    aiedit::PatternDetector pattern_detector(pattern_length, bf, model);
    btllib::SeqReader reader(args.assembly_path, btllib::SeqReader::Flag::LONG_MODE);
    aiedit::VCFWriter vcf_file(vcf_file_path, args.assembly_path);
    btllib::SeqWriter writer(edited_file_path, btllib::SeqWriter::FASTA);

    cli.start_timer("Detecting and correcting errors");
    unsigned num_patterns = 0;
    unsigned num_mismatches = 0;
    const unsigned num_hashes = bf.get_hash_num_per_seed();
    for (auto record : reader) {
        const unsigned chunk_size = record.seq.size() / args.num_threads;
        std::string& seq = record.seq;
#pragma omp parallel for
        for (unsigned i = 0; i < args.num_threads; i++) {
            const unsigned begin = i * chunk_size;
            const unsigned end = i < args.num_threads - 1 ? (i + 1) * chunk_size : seq.size();
            aiedit::SequenceIterator seq_iter(seq, bf.get_seeds(), num_hashes, begin, end);
            aiedit::ErrorDetector err_detector(seq_iter, bf);
            while (err_detector.find_next()) {
                const auto pattern = pattern_detector.get_pattern(seq_iter);
                const bool fixed = mismatch_corrector.fix(seq_iter, pattern);
                num_patterns += fixed ? 1 : 0;
                seq_iter.next(fixed ? 0 : bf.get_k() + pattern_length);
                const unsigned pos = seq_iter.get_position();
                const unsigned seq_len = seq_iter.get_sequence().size();
#pragma omp critical
                cli.log_edit(record.id, fixed, pos, seq_len);
            }
        }
        vcf_file.write(record.id, record.comment, mismatch_corrector.get_edits());
        num_mismatches += mismatch_corrector.get_edits().size();
        mismatch_corrector.clear_edits();
        writer.write(record.id, record.comment, seq);
    }
    cli.stop_timer();
    aiedit::CommandLineInterface::print_num_edits(num_patterns, num_mismatches);

    return 0;
}