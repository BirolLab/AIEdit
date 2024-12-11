#include <btllib/seq_reader.hpp>
#include <iostream>
#include <memory>
#include <omp.h>
#include <string>

#include "core/aiedit.hpp"
#include "program/args.hpp"
#include "program/colorize.hpp"
#include "program/str_utils.hpp"
#include "program/timer.hpp"

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

    omp_set_num_threads(args.num_threads);
    if (omp_get_num_threads() > 1) {
        std::cout << "Multithreading: " << colorize::green("ON") << std::endl;
        std::cout << "- Using " << omp_get_num_threads() << " threads" << std::endl;
        const auto thread_mode = args.contig_mode ? "CONTIGS" : "SEQUENCE CHUNKS";
        std::cout << "- Distributing " << thread_mode << " between threads" << std::endl;
    } else {
        std::cout << "Multithreading: " << colorize::red("OFF") << std::endl;
    }
    std::cout << std::endl;

    Timer timer;

    std::cout << "Loading..." << std::endl;
    std::cout << "- Spaced seeds list from " << args.seeds_path << std::endl;
    std::cout << "- Histogram model from " << args.probs_path << std::endl;
    std::cout << "- AIEdit model from " << args.model_path << std::endl;
    std::cout << "- Bloom filter from " << args.cbf_path << std::endl;
    timer.start();
    std::unique_ptr<aiedit::AIEdit> editor_ptr;
    try {
        editor_ptr = std::make_unique<aiedit::AIEdit>(args.cbf_path,
                                                      args.probs_path,
                                                      args.seeds_path,
                                                      args.model_path);
    } catch (const std::runtime_error& err) {
        std::cerr << colorize::red("Error: ");
        std::cerr << err.what() << std::endl;
        return EXIT_FAILURE;
    }
    auto& editor = (*editor_ptr);
    std::cout << "- " << colorize::green("Done in ", timer.stop(), "s") << std::endl;
    std::cout << "- CBF size: " << str_utils::human_readable(editor.get_cbf_size()) << std::endl;
    std::cout << "- Number of seeds: " << colorize::green(editor.get_num_seeds()) << std::endl;
    std::cout << "- Maximum k-mer length: " << colorize::green(editor.get_max_k()) << std::endl;
    std::cout << "- Maximum indel length: " << editor.get_max_indels() << std::endl;
    std::cout << std::endl;

    std::cout << "Finding edits in " << args.in_path << "..." << std::endl;
    btllib::SeqReader seq_reader(args.in_path, btllib::SeqReader::Flag::LONG_MODE);
#pragma omp parallel
    for (auto record : seq_reader) {
        std::cout << record.id << std::endl;
        editor.get_edits(record.seq, 0, record.seq.size());
    }

    return EXIT_SUCCESS;
}