#include <mpi.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>

#include "grep.h"

void grep::get_lines(std::vector<std::string> &input_string, const std::string &file_name)
{
    std::ifstream f_stream(file_name);
    for (std::string line; std::getline(f_stream, line);)
    {
        input_string.push_back(line);
    }
    f_stream.close();
}

void grep::split_lines(std::vector<std::string> &input_string, unsigned &local_lines_number)
{
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Send total number of lines to all
    unsigned nLinesTotal = input_string.size();
    MPI_Bcast(&nLinesTotal, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    // Split lines evenly
    std::vector<unsigned> nLinesLocal(size, nLinesTotal / size);
    std::vector<int> sendcounts(size), displs(size, 0);
    std::vector<unsigned> local_lines_numbers(size, 1);
    for (unsigned p = 0; p < size; p++)
    {
        // Add orphan elements
        if (p < (nLinesTotal % size))
        {
            nLinesLocal[p] += 1;
        }

        // Compute sendcounts and displacements
        sendcounts[p] = (grep::LINELENGTH + 1) * nLinesLocal[p];
        if (p > 0)
        {
            displs[p] = displs[p - 1] + sendcounts[p - 1];
        }

        // Compute local line numbers
        if (p > 0)
        {
            local_lines_numbers[p] = local_lines_numbers[p - 1] + nLinesLocal[p - 1];
        }
    }

    // Save the number of lines that the process will receive
    local_lines_number = local_lines_numbers[rank];

    // Create the long string to send
    std::string linesString;
    if (rank == 0)
    {
        for (int l = 0; l < input_string.size(); l++)
        {
            std::string line = input_string[l];
            if (line.length() < (grep::LINELENGTH + 1))
            {
                line.insert(line.length(), (grep::LINELENGTH + 1) - line.length(), 0x00);
            }
            linesString.append(line);
        }
        input_string.clear();
    }

    // Send lines
    char linesLocal[sendcounts[rank]];
    MPI_Scatterv(
        &linesString[0],
        &sendcounts[0],
        &displs[0],
        MPI_CHAR,
        &linesLocal[0],
        sendcounts[rank],
        MPI_CHAR,
        0,
        MPI_COMM_WORLD);

    // Save lines in local_lines
    for (unsigned l = 0; l < nLinesLocal[rank]; l++)
    {
        char lineCharArray[(grep::LINELENGTH + 1)];
        std::strncpy(lineCharArray, &linesLocal[l * (grep::LINELENGTH + 1)], (grep::LINELENGTH + 1));
        input_string.push_back(lineCharArray);
    }
}

void grep::search_string(const std::vector<std::string> &input_strings, const std::string &search_string, grep::lines_found &lines, unsigned &local_lines_number)
{
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    for (int l = 0; l < input_strings.size(); l++)
    {
        std::size_t found = input_strings[l].find(search_string);
        if (found != std::string::npos)
        {
            grep::number_and_line line_found(local_lines_number + l, input_strings[l]);
            lines.push_back(line_found);
        }
    }
}

void grep::print_result(const grep::lines_found &lines, unsigned local_lines_number)
{
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Gather number of lines found for each process
    unsigned nFoundLocal = lines.size();
    int nFound[size];
    MPI_Gather(&nFoundLocal, 1, MPI_UNSIGNED, &nFound[0], 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Prepare recvcounts and displs
    unsigned nFoundTotal;
    std::vector<int> recvcounts(size), lineDispls(size, 0), numberDispls(size, 0);
    if (rank == 0)
    {
        for (unsigned p = 0; p < size; p++)
        {
            nFoundTotal += nFound[p];
            recvcounts[p] = (grep::LINELENGTH + 1) * nFound[p];
            if (p > 0)
            {
                lineDispls[p] = lineDispls[p - 1] + recvcounts[p - 1];
                numberDispls[p] = numberDispls[p - 1] + nFound[p - 1];
            }
        }
    }

    // Gather the lines number of the found lines
    unsigned localLinesNumbers[nFoundLocal], allLinesNumbers[nFoundTotal];
    for (unsigned l = 0; l < nFoundLocal; l++)
    {
        localLinesNumbers[l] = lines[l].first;
    }

    MPI_Gatherv(&localLinesNumbers[0], nFoundLocal, MPI_UNSIGNED, &allLinesNumbers[0], &nFound[0], &numberDispls[0], MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    // Gather the lines
    std::string linesString;
    for (int l = 0; l < nFoundLocal; l++)
    {
        grep::number_and_line n_and_line = lines[l];
        std::string line = n_and_line.second;
        if (line.length() < (grep::LINELENGTH + 1))
        {
            line.insert(line.length(), (grep::LINELENGTH + 1) - line.length(), 0x00);
        }
        linesString.append(line);
    }

    char allFoundLines[nFoundTotal * (grep::LINELENGTH + 1)];
    MPI_Gatherv(&linesString[0], nFoundLocal * (grep::LINELENGTH + 1), MPI_CHAR, &allFoundLines[0], &recvcounts[0], &lineDispls[0], MPI_CHAR, 0, MPI_COMM_WORLD);

    // Print found lines
    if (rank == 0)
    {
        std::ofstream f_stream(grep::OUTPUT_FILE);
        for (unsigned p = 0; p < size; p++)
        {
            for (unsigned l = 0; l < nFound[p]; l++)
            {
                char lineCharArray[(grep::LINELENGTH + 1)];
                std::strncpy(lineCharArray, &allFoundLines[lineDispls[p] + l * (grep::LINELENGTH + 1)], (grep::LINELENGTH + 1));
                f_stream << allLinesNumbers[numberDispls[p] + l] << ":" << lineCharArray << std::endl;
            }
        }
        f_stream.close();
    }
}