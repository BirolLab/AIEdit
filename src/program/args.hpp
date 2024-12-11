#pragma once

#include <iostream>
#include <string>

class ProgramArguments
{
  public:

    std::string in_path;
    std::string cbf_path;
    std::string probs_path;
    std::string seeds_path;
    std::string model_path;
    std::string out_path;
    unsigned num_threads;
    bool contig_mode;
    bool no_apply;
    bool verbose;

    void parse(int argc, char** argv);
    void print_help(std::ostream& stream);

  private:

    std::string help_message;
};