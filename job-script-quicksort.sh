#!/bin/bash
#SBATCH --job-name add_matrix_serial
#SBATCH -q secondary
#SBATCH -N 1
#SBATCH --ntasks-per-node=1
#SBATCH --mem=12G
#SBATCH --constraint=amd
#SBATCH -o output_%j.out
#SBATCH -e errors_%j.err
#SBATCH -t 7-0:0:0

./quicksort 24

