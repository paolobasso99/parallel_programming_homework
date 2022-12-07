#ifndef GREP_H
#define GREP_H

#include <vector>
#include <string>
#include <utility>

namespace grep
{
    const int LINELENGTH = 80;
    const std::string OUTPUT_FILE = "output-mpi.txt";

    /**
     * Get the lines from the file and split them between processes.
     * 
     * Args:
     *  all_lines (std::vector<std::string>): Where process with rank 0 will store all the lines found.
     *  local_lines (std::vector<std::string>): Where every process will have their local lines.
     *  file_name (const std::string): The filename.
     *  local_lines_start_from (unsigned): From which number the local lines starts.
    */
    void get_lines(
        std::vector<std::string> &all_lines,
        std::vector<std::string> &local_lines,
        const std::string &file_name,
        unsigned &local_lines_start_from);

    /**
     * Search the local lines to find the specified string and save the number of the line
     * where the string is found.
     * 
     * Args:
     *  local_lines (const std::vector<std::string>): The local lines of the process.
     *  search_string (const std::string): The string to search.
     *  local_matching_numbers (std::vector<unsigned>): Where to save the numbers of the matching lines.
     *  local_lines_start_from (const unsigned): From which number the local lines starts.
    */
    void search_string(
        const std::vector<std::string> &local_lines,
        const std::string &search_string,
        std::vector<unsigned> &local_matching_numbers,
        const unsigned &local_lines_start_from);

    /**
     * From the local numbers of the matching lines print all the matching lines.
     * 
     * Args:
     *  all_lines (const std::vector<std::string>): Where process with rank 0 will store all the lines found.
     *  local_matching_numbers (const std::vector<unsigned>): The numbers of the matching local lines.
    */
    void print_result(
        const std::vector<std::string> &all_lines,
        const std::vector<unsigned> &local_matching_numbers);
}

#endif // GREP_H