#include <mpi.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>

#include "grep.h"

void grep::get_lines(std::vector<std::string> &input_string, const std::string &file_name)
{
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Read file and find number of lines
    unsigned nLinesTotal = 0;
    std::string lines;
    if (rank == 0)
    {
        std::ifstream f_stream(file_name);
        for (std::string line; std::getline(f_stream, line);)
        {
            ++nLinesTotal;
            if (line.length() < grep::LINELENGTH)
            {
                line.insert(line.length(), grep::LINELENGTH - line.length(), 0x00);
            }
            lines.append(line);
        }
        f_stream.close();
    }

    // Send total number of lines to all
    MPI_Bcast(&nLinesTotal, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    // Split lines evenly
    std::vector<unsigned> nLinesLocal(size, nLinesTotal / size);
    std::vector<int> sendcounts(size);
    std::vector<int> displs(size, 0);
    for (unsigned p = 0; p < size; p++)
    {
        if (p < (nLinesTotal % size))
        {
            nLinesLocal[p] += 1;
        }
        sendcounts[p] = grep::LINELENGTH * nLinesLocal[p];
        if (p > 0)
        {
            displs[p] = displs[p - 1] + sendcounts[p - 1];
        }
    }

    // Send lines
    char linesLocal[sendcounts[rank]];
    MPI_Scatterv(
        &lines[0],
        &sendcounts[0],
        &displs[0],
        MPI_CHAR,
        &linesLocal[0],
        sendcounts[rank],
        MPI_CHAR,
        0,
        MPI_COMM_WORLD);

    // Save lines in input_string
    for (unsigned l = 0; l < nLinesLocal[rank]; l++)
    {
        char lineCharArray[grep::LINELENGTH];
        std::strncpy(lineCharArray, &linesLocal[l*grep::LINELENGTH], grep::LINELENGTH);
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
            grep::number_and_line line_found(l, input_strings[l]);
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
    unsigned nFound[size];
    MPI_Gather(&nFoundLocal, 1, MPI_UNSIGNED, &nFound[0], 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    // Gather the found lines
    unsigned nFoundTotal;
    std::vector<int> recvcounts(size);
    std::vector<int> displs(size, 0);
    if (rank == 0)
    {
        for (unsigned p = 0; p < size; p++) 
        {
            nFoundTotal += nFound[p];
            recvcounts[p] = grep::LINELENGTH * nFound[p];
            if (p > 0)
            {
                displs[p] = displs[p - 1] + recvcounts[p - 1];
            }
        }
    }

    if (rank == 0)
    {
        for (unsigned p = 0; p < size; p++)
        {
            std::cout << "Process " << p << " will send " << nFound[p] << " lines, " << recvcounts[p] << " chars, starting from " << displs[p] << std::endl;
        }
    }

    // Create the long string to send
    std::string linesString;
    for (int l = 0; l < nFoundLocal; l++) 
    {
        grep::number_and_line n_and_line = lines[l];
        std::string line = n_and_line.second;
        if (line.length() < grep::LINELENGTH)
        {
            line.insert(line.length(), grep::LINELENGTH - line.length(), 0x00);
        }
        linesString.append(line);
    }

    char allFoundLines[nFoundTotal*grep::LINELENGTH];
    MPI_Gatherv(&linesString[0], nFoundLocal*grep::LINELENGTH, MPI_CHAR, &allFoundLines[0], &recvcounts[0], &displs[0], MPI_CHAR, 0, MPI_COMM_WORLD);

    // Print found lines
    if (rank == 0)
    {
        std::cout << allFoundLines << std::endl;
        std::cout << "------------------------------" << std::endl;
        for (unsigned p = 0; p < size; p++)
        {
            for (unsigned l = 0; l < nFound[p]; l++)
            {
                char lineCharArray[grep::LINELENGTH];
                std::strncpy(lineCharArray, &allFoundLines[displs[p] + l*grep::LINELENGTH], grep::LINELENGTH);
                std::cout << p << "," << l << ":" << lineCharArray << std::endl;
            }
        }
    }
}