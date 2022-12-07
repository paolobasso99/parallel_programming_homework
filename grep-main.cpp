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
    double init_time = MPI_Wtime();

    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::vector<std::string> all_lines, local_lines;
    unsigned local_lines_start_from;
    std::vector<unsigned> local_matching_numbers;

    grep::get_lines(all_lines, local_lines, argv[2], local_lines_start_from);
    grep::search_string(local_lines, argv[1], local_matching_numbers, local_lines_start_from);
    grep::print_result(all_lines, local_matching_numbers);

    if (rank == 0) {
        std::cout << "Enlapsed time: " <<  (MPI_Wtime() - init_time)*1000 << "ms" << std::endl;
    }

    MPI_Finalize();
    return 0;
}