for (unsigned l = 0; l < input_lines.size(); l++)
    {
        std::cout << "p" << rank << " " << l + local_lines_number << ":" << input_lines[l] << std::endl;
    }