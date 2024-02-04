# Odd-Even Sort MPI

## Overview
This project implements the odd-even sort algorithm using the Message Passing Interface (MPI) for parallel computing. The odd-even sort is a comparison-based sorting algorithm, primarily used for sorting a list of elements into a specified order (ascending or descending). By utilizing MPI, this implementation aims to achieve faster sorting times on large datasets by distributing the workload across multiple processors.

## Features
- Implementation of the odd-even sort algorithm using MPI.
- Efficient sorting of large datasets.
- Scalability across multiple processors.

## Prerequisites
Before you begin, ensure you have met the following requirements:
- MPI library installed (e.g., MPICH, Open MPI).
- C compiler (e.g., GCC).

## Installation
To install `odd-even-sort-mpi`, follow these steps:
1. Clone the repository:
```
git clone https://github.com/riddheshMarkandeya/odd-even-sort-mpi.git
```
2. Navigate to the project directory:
```
cd odd-even-sort-mpi
```

## Usage
To run `odd-even-sort-mpi`, follow these steps:
1. Compile the program using an MPI compiler:
```
mpicc -o odd-even-sort odd-even-sort.c
```
2. Run the program using `mpiexec` or `mpirun`, specifying the number of processes:
```
mpiexec -n <number_of_processes> ./odd-even-sort
```
Replace `<number_of_processes>` with the desired number of processes.

## Example
Running the program with 4 processes:
```
mpiexec -n 4 ./odd-even-sort
```
