# Parallel programming homewrok: parallel `grep` using MPI
This is my solution to an homework done at the PARALLEL PROGRAMMING course at Politecnico di Milano in 2022.

The prompt was to build a parallel version of the `grep` program using [MPI](https://www.open-mpi.org/).
The homework prompt is aviable in [Grep_Homework_Question notebook](Grep_Homework_Question.ipynb) while my solution is in the[Grep_Homework_Solution notebook](Grep_Homework_Solution.ipynb).

You can open the solution directly on colab: [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/paolobasso99/parallel_programming_homework/blob/main/Grep_Homework_Solution_Colab.ipynb)

## Setup
This homework is in C++ so you need a development envirorment that supports it, if you need help I found [this good guide](https://code.visualstudio.com/docs/languages/cpp).

You also need to install MPI.

To run the notebooks you need to have Python installed.

## Compile and run
To compile use:
```bash
mpicxx -fdiagnostics-color=always -g src/grep-main.cpp src/grep.cpp -o dest/grep-mpi.exe
mpicxx -fdiagnostics-color=always -g src/grep-sequential.cpp -o dest/grep-sequential.exe
```

And to run:
```bash
mpiexec -np 1 dest/grep-sequential matter input_file.txt
mpiexec -np 8 dest/grep-mpi matter input_file.txt
```

## Project structure
The following files and folder are present:
- [Grep_Homework_Question.ipynb](Grep_Homework_Question.ipynb): contains the homework prompt.
- [Grep_Homework_Solution.ipynb](Grep_Homework_Solution.ipynb): contains my solution.
- [input_file.txt](input_file.txt): The input file to test the program
- [src/](src/): Folder containing `cpp` source files.
  - [grep-sequential.cpp](src/grep-sequential.cpp): A sequential implementation of grep which also measures the enlapsed time.
  - [grep-main.cpp](src/grep-main.cpp): The `main` function of the parallel implementation.
  - [grep.cpp](src/grep.cpp): The implementation `grep-mpi` helper functions the parallel implementation.
  - [grep.h](src/grep.h): The header file of `grep-mpi` helper functions the parallel implementation.
- [outputs/](outputs/): Folder which will contain the outputs.
  - [output-correct.txt](outputs/output-correct.txt): The correct output of the normal `grep`.
- [dest/](dest/): Folder which will contain the compiled executable files.
- [.vscode/](.vscode/): Folder containing some VS Code configurations.