#ifndef ARGS_HPP
#define ARGS_HPP

#include <filesystem>
#include <string>

class ProgramArguments
{
  public:
    std::filesystem::path assembly_path;
    std::filesystem::path bf_path;
    std::filesystem::path out_path;
    unsigned verbosity;
    unsigned pattern_length;
    unsigned num_threads;

    void parse(int argc, char** argv);
};

#endif // ARGS_HPP