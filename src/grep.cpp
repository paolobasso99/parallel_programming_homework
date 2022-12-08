#include <mpi.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>

#include "grep.h"

void grep::get_lines(
    const unsigned &rank,
    const unsigned &size,
    std::string &all_lines,
    unsigned &total_number_of_lines,
    const std::string &file_name)
{
    if (rank == 0)
    {
        std::ifstream f_stream(file_name);
        unsigned counter = 0;
        for (std::string line; std::getline(f_stream, line);)
        {
            if (line.length() > grep::LINELENGTH)
            {
                std::cout << "There is a line longher than " << grep::LINELENGTH << std::endl;
                exit(EXIT_FAILURE);
            }

            ++counter;

            // Pad with 0x00
            line.insert(line.length(), (grep::LINELENGTH + 1) - line.length(), 0x00);
            all_lines.append(line);
        }
        f_stream.close();
        total_number_of_lines = counter;
    }

    // Send total number of lines to all
    MPI_Bcast(&total_number_of_lines, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
}

void grep::split_lines(
    const unsigned &rank,
    const unsigned &size,
    const std::string &all_lines,
    const unsigned &total_number_of_lines,
    std::vector<std::string> &local_lines,
    unsigned &local_lines_start_from)
{
    // Split lines evenly between processes
    std::vector<unsigned> local_number_of_lines(size, total_number_of_lines / size);
    std::vector<int> sendcounts(size), displs(size, 0);
    std::vector<unsigned> local_lines_start_from_vector(size, 1);
    for (unsigned p = 0; p < size; p++)
    {
        // Add orphan elements
        if (p >= (total_number_of_lines % size))
        {
            local_number_of_lines[p] += 1;
        }

        // Compute sendcounts and displacements
        sendcounts[p] = (grep::LINELENGTH + 1) * local_number_of_lines[p];
        if (p > 0)
        {
            displs[p] = displs[p - 1] + sendcounts[p - 1];
        }

        // Compute local line numbers
        if (p > 0)
        {
            local_lines_start_from_vector[p] = local_lines_start_from_vector[p - 1] + local_number_of_lines[p - 1];
        }
    }

    // Save from where each process lines will start
    local_lines_start_from = local_lines_start_from_vector[rank];

    // Send lines
    char local_lines_char_array[sendcounts[rank]];
    MPI_Scatterv(
        &all_lines[0],
        &sendcounts[0],
        &displs[0],
        MPI_CHAR,
        &local_lines_char_array[0],
        sendcounts[rank],
        MPI_CHAR,
        0,
        MPI_COMM_WORLD);

    // Save lines in local_lines
    char line_as_char_array[(grep::LINELENGTH + 1)];
    for (unsigned l = 0; l < local_number_of_lines[rank]; l++)
    {
        std::strncpy(line_as_char_array, &local_lines_char_array[l * (grep::LINELENGTH + 1)], (grep::LINELENGTH + 1));
        local_lines.push_back(line_as_char_array);
    }
}

void grep::search_string(
    const unsigned &rank,
    const unsigned &size,
    const std::vector<std::string> &local_lines,
    const std::string &search_string,
    std::vector<unsigned> &local_matching_numbers,
    const unsigned &local_lines_start_from)
{
    std::size_t found;
    for (int l = 0; l < local_lines.size(); l++)
    {
        found = local_lines[l].find(search_string);
        if (found != std::string::npos)
        {
            local_matching_numbers.push_back(local_lines_start_from + l);
        }
    }
}

void grep::print_result(
    const unsigned &rank,
    const unsigned &size,
    const std::string &all_lines,
    const std::vector<unsigned> &local_matching_numbers)
{
    // Gather number of lines found by each process
    unsigned local_number_of_filtered = local_matching_numbers.size();
    int number_of_filtered_array[size];
    MPI_Gather(&local_number_of_filtered, 1, MPI_UNSIGNED, &number_of_filtered_array[0], 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Count total number of lines found and prepare displs
    unsigned total_number_of_filtered = 0;
    std::vector<int> displs(size, 0);
    if (rank == 0)
    {
        for (unsigned p = 0; p < size; p++)
        {
            total_number_of_filtered += number_of_filtered_array[p];
            if (p > 0)
            {
                displs[p] = displs[p - 1] + number_of_filtered_array[p - 1];
            }
        }
    }

    // Gather the line numbers of the found lines
    unsigned all_numbers_filtered[total_number_of_filtered];
    MPI_Gatherv(
        &local_matching_numbers[0],
        local_number_of_filtered,
        MPI_UNSIGNED,
        &all_numbers_filtered[0],
        &number_of_filtered_array[0],
        &displs[0],
        MPI_UNSIGNED,
        0,
        MPI_COMM_WORLD);

    // Print found lines with all_lines[all_numbers_filtered[i]]
    if (rank == 0)
    {
        std::ofstream f_stream(grep::OUTPUT_FILE);
        char line_as_char_array[(grep::LINELENGTH + 1)];
        for (unsigned n = 0; n < total_number_of_filtered; n++)
        {
            std::strncpy(
                line_as_char_array,
                &all_lines[(all_numbers_filtered[n] - 1) * (grep::LINELENGTH + 1)],
                (grep::LINELENGTH + 1));
            f_stream << all_numbers_filtered[n] << ":" << line_as_char_array << std::endl;
        }
        f_stream.close();
    }
}