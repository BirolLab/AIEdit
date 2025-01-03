#include <btllib/seq_reader.hpp>
#include <btllib/seq_writer.hpp>
#include <filesystem>
#include <iostream>
#include <memory>
#include <omp.h>
#include <string>

#include "core/aiedit.hpp"
#include "program/args.hpp"
#include "program/colorize.hpp"
#include "program/file_io.hpp"
#include "program/str_utils.hpp"
#include "program/timer.hpp"

std::unique_ptr<aiedit::AIEdit> initialize(const ProgramArguments& args)
{
    try {
        return std::make_unique<aiedit::AIEdit>(args.cbf_path,
                                                args.probs_path,
                                                args.seeds_path,
                                                args.model_path,
                                                args.contig_mode ? 1 : args.num_threads);
    } catch (const std::runtime_error& err) {
        std::cerr << colorize::red("Error: ");
        std::cerr << err.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void write_logs(const std::string& seq_id,
                const aiedit::Results& results,
                EditsListWriter& edits_writer,
                IgnoredPatternsWriter& ignored_writer)
{
#pragma omp critical
    {
        std::cout << "[" << seq_id << "] Applied " << results.edits.size() << " edits, ";
        std::cout << "ignored " << results.ignored.size() << " patterns" << std::endl;
    }
    for (const auto& edit : results.edits) {
#pragma omp critical
        edits_writer.write(seq_id, edit);
    }
    for (const auto& pattern : results.ignored) {
#pragma omp critical
        ignored_writer.write(seq_id, pattern);
    }
}

int main(int argc, char** argv)
{
    ProgramArguments args;
    try {
        args.parse(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << colorize::red("Argument error: ");
        std::cerr << err.what() << std::endl;
        args.print_help(std::cerr);
        return EXIT_FAILURE;
    }

    std::cout << aiedit::LOGO << std::endl;
    std::cout << "Version " << aiedit::VERSION << std::endl << std::endl;

    omp_set_max_active_levels(2);
    if (args.num_threads > 1) {
        std::cout << "Multithreading: " << colorize::green("ON") << std::endl;
        std::cout << "- Using " << args.num_threads << " threads" << std::endl;
        const auto thread_mode = args.contig_mode ? "CONTIGS" : "SEQUENCE CHUNKS";
        std::cout << "- Distributing " << thread_mode << std::endl;
    } else {
        std::cout << "Multithreading: " << colorize::red("OFF") << std::endl;
    }
    std::cout << std::endl;

    Timer timer;

    std::cout << "Loading..." << std::endl;
    std::cout << "- Spaced seeds list: " << args.seeds_path << std::endl;
    std::cout << "- Histogram model: " << args.probs_path << std::endl;
    std::cout << "- AIEdit model: " << args.model_path << std::endl;
    std::cout << "- Bloom filter: " << args.cbf_path << std::endl;
    timer.start();
    auto& editor = (*initialize(args));
    std::cout << "- " << colorize::green("Done in ", timer.stop(), "s") << std::endl;
    std::cout << "- CBF size: " << str_utils::human_readable(editor.get_cbf_size()) << std::endl;
    std::cout << "- Number of seeds: " << colorize::green(editor.get_num_seeds()) << std::endl;
    std::cout << "- Maximum k-mer length: " << colorize::green(editor.get_max_k()) << std::endl;
    std::cout << "- Maximum indel length: " << editor.get_max_indels() << std::endl;
    std::cout << std::endl;

    const std::string prefix = args.out_path / std::filesystem::path(args.in_path).stem();
    btllib::SeqWriter seq_writer(prefix + "_edited.fa", btllib::SeqWriter::FASTA);
    EditsListWriter edits_writer(prefix + "_edits.tsv");
    IgnoredPatternsWriter ignored_writer(prefix + "_ignored.tsv");

    std::cout << "Finding edits in " << args.in_path << "..." << std::endl;
    btllib::SeqReader seq_reader(args.in_path, btllib::SeqReader::Flag::LONG_MODE);
#pragma omp parallel num_threads(args.contig_mode ? args.num_threads : 1)
    for (auto record : seq_reader) {
        const auto results = editor.get_edits(record.seq);
        const auto edited = aiedit::apply_edits(record.seq, results.edits);
        seq_writer.write(record.id, record.comment, edited);
        write_logs(record.id, results, edits_writer, ignored_writer);
    }

    return EXIT_SUCCESS;
}