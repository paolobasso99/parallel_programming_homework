#include <mpi.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>

#include "grep.h"

void grep::get_lines(
    std::vector<std::string> &all_lines,
    std::vector<std::string> &local_lines,
    const std::string &file_name,
    unsigned &local_lines_start_from)
{
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Read file, count lines and build long string of concatenated lines, seprated by 0x00
    std::string linesString;
    unsigned nLinesTotal = 0;
    if (rank == 0)
    {
        std::ifstream f_stream(file_name);
        for (std::string line; std::getline(f_stream, line);)
        {
            ++nLinesTotal;
            all_lines.push_back(line);

            // Pad with 0x00
            if (line.length() < (grep::LINELENGTH + 1))
            {
                line.insert(line.length(), (grep::LINELENGTH + 1) - line.length(), 0x00);
            }
            linesString.append(line);
        }
        f_stream.close();
    }

    // Send total number of lines to all
    MPI_Bcast(&nLinesTotal, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    // Split lines evenly between processes
    std::vector<unsigned> nLinesLocal(size, nLinesTotal / size);
    std::vector<int> sendcounts(size), displs(size, 0);
    std::vector<unsigned> local_local_lines_start_from_vector(size, 1);
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
            local_local_lines_start_from_vector[p] = local_local_lines_start_from_vector[p - 1] + nLinesLocal[p - 1];
        }
    }

    // Save from where each processor lines will start
    local_lines_start_from = local_local_lines_start_from_vector[rank];

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
        local_lines.push_back(lineCharArray);
    }
}

void grep::search_string(
    const std::vector<std::string> &local_lines,
    const std::string &search_string,
    grep::lines_found &local_lines_filtered,
    unsigned &local_lines_start_from)
{
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    for (int l = 0; l < local_lines.size(); l++)
    {
        std::size_t found = local_lines[l].find(search_string);
        if (found != std::string::npos)
        {
            grep::number_and_line line_found(local_lines_start_from + l, local_lines[l]);
            local_lines_filtered.push_back(line_found);
        }
    }
}

void grep::print_result(const grep::lines_found &local_lines_filtered, unsigned local_lines_start_from)
{
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Gather number of lines found for each process
    unsigned nFoundLocal = local_lines_filtered.size();
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
        localLinesNumbers[l] = local_lines_filtered[l].first;
    }

    MPI_Gatherv(
        &localLinesNumbers[0],
        nFoundLocal,
        MPI_UNSIGNED,
        &allLinesNumbers[0],
        &nFound[0],
        &numberDispls[0],
        MPI_UNSIGNED,
        0,
        MPI_COMM_WORLD);

    // Gather the lines
    std::string linesString;
    for (int l = 0; l < nFoundLocal; l++)
    {
        grep::number_and_line n_and_line = local_lines_filtered[l];
        std::string line = n_and_line.second;
        if (line.length() < (grep::LINELENGTH + 1))
        {
            line.insert(line.length(), (grep::LINELENGTH + 1) - line.length(), 0x00);
        }
        linesString.append(line);
    }

    char allFoundLines[nFoundTotal * (grep::LINELENGTH + 1)];
    MPI_Gatherv(
        &linesString[0],
        nFoundLocal * (grep::LINELENGTH + 1),
        MPI_CHAR,
        &allFoundLines[0],
        &recvcounts[0],
        &lineDispls[0],
        MPI_CHAR,
        0,
        MPI_COMM_WORLD);

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