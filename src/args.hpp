#ifndef ARGS_HPP
#define ARGS_HPP

#include <filesystem>
#include <string>

namespace aiedit {

class ProgramArguments
{
  public:

    std::filesystem::path assembly_path;
    std::filesystem::path bf_path;
    std::filesystem::path model_path;
    std::filesystem::path out_path;
    bool verbose;
    bool contig_mode;
    unsigned num_threads;

    void parse(int argc, char** argv);
};

}  // namespace aiedit

#endif  // ARGS_HPP