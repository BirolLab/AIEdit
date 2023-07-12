#ifndef ARGS_HPP
#define ARGS_HPP

#include <filesystem>
#include <string>

namespace aiedit {

class ProgramArguments
{
  public:

    std::filesystem::path in_path;
    std::filesystem::path bf_path;
    std::filesystem::path model_path;
    std::filesystem::path out_path;
    unsigned num_threads;
    bool contig_mode;
    bool no_apply;
    bool verbose;

    /**
     * Parse command-line arguments
     * @param argc Number of input arguments given to the main(...) function
     * @param argv Argument data from the main(...) function
     * @return Arguments object containing parsed arguments
     */
    void parse(int argc, char** argv);

    /**
     * Get the parameter list's help message text
     */
    const std::string get_help_message() const;

  private:

    std::string help_message;
};

}  // namespace aiedit

#endif  // ARGS_HPP