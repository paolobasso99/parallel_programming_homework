#ifndef GREP_H
#define GREP_H

#include <vector>
#include <string>
#include <utility>

namespace grep
{
    const int LINELENGTH = 80;
    const std::string OUTPUT_FILE = "output.txt";

    /* Only process with rank 0 should read from the file,
     * other processes must get their lines from rank 0
     */
    void get_lines(
        std::vector<std::string> &all_lines,
        std::vector<std::string> &local_lines,
        const std::string &file_name,
        unsigned &local_lines_start_from);

    /* Differently from the example seen at lecture, the first input to this
     * function is a vector containing the portion of file that must be searched
     * by each process
     */
    void search_string(
        const std::vector<std::string> &local_lines,
        const std::string &search_string,
        std::vector<unsigned> &local_numbers_filtered,
        const unsigned &local_lines_start_from);

    /* Prints (preferrably to file) must be performed by rank 0 only, it is
     * fine to hard-code the file path for the result in this function */
    void print_result(
        const std::vector<std::string> &all_lines,
        const std::vector<unsigned> &local_numbers_filtered);
}

#endif // GREP_H