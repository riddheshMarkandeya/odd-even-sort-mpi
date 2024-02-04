#!/bin/bash
#SBATCH --job-name add_matrix_mpi
#SBATCH -q secondary
#SBATCH -N 1
#SBATCH --ntasks-per-node=8
#SBATCH --mem=12G
#SBATCH --constraint=amd
#SBATCH -o output_%j.out
#SBATCH -e errors_%j.err
#SBATCH -t 7-0:0:0


module load openmpi3/3.1.0
mpirun -np 8 --mca btl ^openib ./original_odd_even_sort_mpi 24

