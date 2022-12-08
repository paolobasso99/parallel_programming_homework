#ifndef GREP_H
#define GREP_H

#include <vector>
#include <string>

namespace grep
{
    const int LINELENGTH = 80;
    const std::string OUTPUT_FILE = "outputs/output-mpi.txt";

    /**
     * Get the lines from the file and split them between processes.
     *
     * Args:
     *  rank (const unsigned): The rank of the process executing the function.
     *  size (const unsigned): The size of the communication channel.
     *  all_lines (std::string): Where process with rank 0 will store all the lines, concatenated
     *      one every (LINELENGTH + 1) characters (+1 is for null terminator), padded with null terminators.
     *  total_number_of_lines (unsigned): Where all the process will have the total number of lines in the file.
     *  file_name (const std::string): The filename.
     */
    void get_lines(
        const unsigned &rank,
        const unsigned &size,
        std::string &all_lines,
        unsigned &total_number_of_lines,
        const std::string &file_name);

    /**
     * Split the lines evenly between processes.
     *
     * Args:
     *  rank (const unsigned): The rank of the process executing the function.
     *  size (const unsigned): The size of the communication channel.
     *  all_lines (const std::string): Where process with rank 0 will store all the lines, concatenated
     *      one every (LINELENGTH + 1) characters (+1 is for null terminator), padded with null terminators.
     *  total_number_of_lines (const unsigned): Total number of lines in the file.
     *  local_lines (std::vector<std::string>): Where every process will have their local lines.
     *  local_lines_start_from (unsigned): From which number the local lines starts.
     */
    void split_lines(
        const unsigned &rank,
        const unsigned &size,
        const std::string &all_lines,
        const unsigned &total_number_of_lines,
        std::vector<std::string> &local_lines,
        unsigned &local_lines_start_from);

    /**
     * Search the local lines to find the specified string and save the number of the line
     * where the string is found.
     *
     * Args:
     *  rank (const unsigned): The rank of the process executing the function.
     *  size (const unsigned): The size of the communication channel.
     *  local_lines (const std::vector<std::string>): The local lines of the process.
     *  search_string (const std::string): The string to search.
     *  local_matching_numbers (std::vector<unsigned>): Where to save the numbers of the matching lines.
     *  local_lines_start_from (const unsigned): From which number the local lines starts.
     */
    void search_string(
        const unsigned &rank,
        const unsigned &size,
        const std::vector<std::string> &local_lines,
        const std::string &search_string,
        std::vector<unsigned> &local_matching_numbers,
        const unsigned &local_lines_start_from);

    /**
     * From the local numbers of the matching lines print all the matching lines.
     *
     * Args:
     *  rank (const unsigned): The rank of the process executing the function.
     *  size (const unsigned): The size of the communication channel.
     *  all_lines (const std::vector<std::string>): Where process with rank 0 has all the lines, concatenated
     *  one every (LINELENGTH + 1) characters (+1 is for null terminator), padded with null terminators.
     *  local_matching_numbers (const std::vector<unsigned>): The numbers of the matching local lines.
     */
    void print_result(
        const unsigned &rank,
        const unsigned &size,
        const std::string &all_lines,
        const std::vector<unsigned> &local_matching_numbers);
}

#endif // GREP_H