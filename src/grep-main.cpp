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

    double start_time = MPI_Wtime();

    std::string all_lines;
    std::vector<std::string> local_lines;
    unsigned local_lines_start_from, total_number_of_lines;
    std::vector<unsigned> local_matching_numbers;

    grep::get_lines(rank, size, all_lines, total_number_of_lines, argv[2]);
    double get_lines_time = MPI_Wtime();
    if (rank == 0)
    {
        std::cout << "Get lines time: " << (get_lines_time - start_time) * 1000 << "ms" << std::endl;
    }

    grep::split_lines(rank, size, all_lines, total_number_of_lines, local_lines, local_lines_start_from);
    double split_lines_time = MPI_Wtime();
    if (rank == 0)
    {
        std::cout << "Split lines time: " << (split_lines_time - get_lines_time) * 1000 << "ms" << std::endl;
    }

    grep::search_string(rank, size, local_lines, argv[1], local_matching_numbers, local_lines_start_from);
    double search_string_time = MPI_Wtime();
    if (rank == 0)
    {
        std::cout << "Search string time: " << (search_string_time - split_lines_time) * 1000 << "ms" << std::endl;
    }

    grep::print_result(rank, size, all_lines, local_matching_numbers);
    double print_results_time = MPI_Wtime();
    if (rank == 0)
    {
        std::cout << "Print results time: " << (print_results_time - search_string_time) * 1000 << "ms" << std::endl;
        std::cout << "Total enlapsed time: " << (print_results_time - start_time) * 1000 << "ms" << std::endl;
    }


    MPI_Finalize();
    return 0;
}
