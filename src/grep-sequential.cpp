#include <iostream>
#include <fstream>
#include <mpi.h>
#include <vector>
#include <string>

const int LINELENGTH = 80;
const std::string OUTPUT_FILE = "outputs/output-sequential.txt";

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Expected 2 inputs, got " << argc - 1 << std::endl;
        return 0;
    }

    MPI_Init(&argc, &argv);
    double start_time = MPI_Wtime();

    // Read imput file and save lines to vector
    std::vector<std::string> lines;
    std::ifstream f_stream_in(argv[2]);
    for (std::string line; std::getline(f_stream_in, line);)
    {
        if (line.length() > LINELENGTH) {
            std::cout << "There is a line longher than " << LINELENGTH << std::endl;
            exit(EXIT_FAILURE);
        }
        lines.push_back(line);
    }
    f_stream_in.close();
    double get_lines_time = MPI_Wtime();
    std::cout << "Get lines time: " << (get_lines_time - start_time) * 1000 << "ms" << std::endl;

    // Find matching line numbers
    unsigned current_line_number = 0;
    std::size_t found;
    std::vector<int> matching_line_numbers;
    for (unsigned l = 0; l < lines.size(); l++)
    {
        ++current_line_number;
        found = lines[l].find(argv[1]);
        if (found != std::string::npos)
        {
            matching_line_numbers.push_back(l+1);
        }
    }
    double search_string_time = MPI_Wtime();
    std::cout << "Search string time: " << (search_string_time - get_lines_time) * 1000 << "ms" << std::endl;

    // Print matching lines to file
    std::ofstream f_stream_out(OUTPUT_FILE);
    for (unsigned n = 0; n < matching_line_numbers.size(); n++)
    {
        f_stream_out << matching_line_numbers[n] << ":" << lines[matching_line_numbers[n] - 1] << std::endl;
    }
    f_stream_out.close();
    double print_results_time = MPI_Wtime();
    std::cout << "Print results time: " << (print_results_time - search_string_time) * 1000 << "ms" << std::endl;
    std::cout << "Total enlapsed time: " <<  (print_results_time - start_time)*1000 << "ms" << std::endl;

    MPI_Finalize();
    return 0;
}
