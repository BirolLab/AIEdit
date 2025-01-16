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

    Timer timer;

    std::cout << "- Spaced seeds list: " << args.seeds_path << std::endl;
    std::cout << "- Histogram model: " << args.probs_path << std::endl;
    std::cout << "- AIEdit model: " << args.model_path << std::endl;
    std::cout << "- Bloom filter: " << args.cbf_path << std::endl;
    std::cout << std::endl;

    std::cout << "Loading..." << std::endl;
    timer.start();
    std::unique_ptr<aiedit::AIEdit> editor_ptr;
    try {
        editor_ptr = std::make_unique<aiedit::AIEdit>(args.cbf_path,
                                                      args.probs_path,
                                                      args.seeds_path,
                                                      args.model_path,
                                                      10,
                                                      args.threshold);
    } catch (const std::runtime_error& err) {
        std::cerr << colorize::red("Error: ");
        std::cerr << err.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    auto& editor = *editor_ptr;
    std::cout << "- " << colorize::green("Done in ", timer.stop(), "s") << std::endl;

    btllib::SeqReader seq_reader(args.in_path, btllib::SeqReader::Flag::LONG_MODE);

    for (auto record : seq_reader) {
        const auto results = editor.train(record.seq);
    }

    return EXIT_SUCCESS;
}