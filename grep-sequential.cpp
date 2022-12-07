#include <iostream>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>

const int LINELENGTH = 80;
const std::string OUTPUT_FILE = "output-sequential.txt";

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Expected 2 inputs, got " << argc - 1 << std::endl;
        return 0;
    }

    std::ofstream f_stream_out(OUTPUT_FILE);
    std::ifstream f_stream_in(argv[2]);
    unsigned current_line_number = 0;

    for (std::string line; std::getline(f_stream_in, line);)
    {
        if (line.length() > LINELENGTH) {
            std::cout << "There is a line longher than " << LINELENGTH << std::endl;
            exit(EXIT_FAILURE);
        }

        ++current_line_number;
        
        std::size_t found = line.find(argv[1]);
        if (found != std::string::npos)
        {
            f_stream_out << current_line_number << ":" << line << std::endl;
        }
    }
    f_stream_in.close();
    f_stream_out.close();

    return 0;
}