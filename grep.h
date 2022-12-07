#ifndef GREP_H
#define GREP_H

#include <vector>
#include <string>
#include <utility>

namespace grep
{
    const int LINELENGTH = 80;
    const std::string OUTPUT_FILE = "output.txt";

    typedef std::pair<unsigned, std::string> number_and_line;

    typedef std::vector<number_and_line> lines_found;

    /* Only process with rank 0 should read from the file,
     * other processes must get their lines from rank 0
     */
    void get_lines(std::vector<std::string> &input_string, const std::string &file_name);

    void split_lines(std::vector<std::string> &input_string, unsigned &local_lines_number);

    /* Differently from the example seen at lecture, the first input to this
     * function is a vector containing the portion of file that must be searched
     * by each process
     */
    void search_string(
        const std::vector<std::string> &input_strings,
        const std::string &search_string, lines_found &lines,
        unsigned &local_lines_number);

    /* Prints (preferrably to file) must be performed by rank 0 only, it is
     * fine to hard-code the file path for the result in this function */
    void print_result(const lines_found &lines, unsigned local_lines_number);
}

#endif // GREP_H