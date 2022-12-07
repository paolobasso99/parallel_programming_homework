#include <mpi.h>
#include <iostream>

#include "grep.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Expected 2 inputs, got " << argc - 1 << std::endl;
        return 0;
    }

    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::vector<std::string> all_lines, local_lines;
    unsigned local_lines_start_from;
    grep::lines_found local_lines_filtered;

    grep::get_lines(all_lines, local_lines, argv[2], local_lines_start_from);
    grep::search_string(local_lines, argv[1], local_lines_filtered, local_lines_start_from);
    grep::print_result(local_lines_filtered, local_lines_start_from);

    MPI_Finalize();
    return 0;
}