#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_VALUE 128

int compareSplit(int nlocal, int *elmnts, int *relmnts, int *wspace, int keepsmall);
void write_file(int *elements, int size);
int random_at_most(int max);
int incOrder(const void *e1, const void *e2);


int main(int argc, char* argv[]) {
  srand(time(0));

  int *elements;
  int size;
  int npes;
  int myrank;
  int nlocal;
  int *local_elements;
  int *recv_elements;
  int *workspace;
  int *p_same_array;
  int oddrank;
  int evenrank;
  int i;
  struct timespec begin, end;
  MPI_Status status;

  /* Initialize MPI and get system information */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &npes);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  if (argc == 1) {
    printf("No argument provided for array size exponent taking 128 as default.\n");
    printf("e.g. Run \"./a.out 10\" for size 2^10.\n");
    size = 128;
  } else {
    size = (int)pow(2, (double)atoi(argv[1]));
    // printf("%d: Array size is %d %d\n", myrank, size, npes);
  }

  if (myrank == 0) {
    elements = (int *)malloc(size * sizeof(int));
    for(i = 0; i < size; i++) {
      elements[i] = random_at_most(MAX_VALUE);
    }
    // printf("all elms ");
    // for (i = 0; i < size; i++) {
    //   printf("%d ", elements[i]);
    // }
    // printf("\n");

    // Not including main array initialization, as I'll exclude that from serial as well.
    clock_gettime(CLOCK_MONOTONIC_RAW, &begin);
  }

  nlocal = size/npes; /* Compute the number of elements to be stored locally. */
  local_elements = (int *)malloc(nlocal * sizeof(int));
  recv_elements = (int *)malloc(nlocal * sizeof(int));
  workspace = (int *)malloc(nlocal * sizeof(int));
  p_same_array = (int *)malloc(npes * sizeof(int));

  /* Determine the rank of the processors that myrank needs to communicate during
  the odd and even phases of the algorithm */
  if (myrank % 2 == 0) {
    oddrank = myrank-1;
    evenrank = myrank+1;
  } else {
    oddrank = myrank+1;
    evenrank = myrank-1;
  }

  /* Set the ranks of the processors at the end of the linear */
  if (oddrank == -1 || oddrank == npes) {
    oddrank = MPI_PROC_NULL;
  }
  if (evenrank == -1 || evenrank == npes) {
    evenrank = MPI_PROC_NULL;
  }

  MPI_Scatter(elements, nlocal, MPI_INT, local_elements, nlocal, MPI_INT, 0, MPI_COMM_WORLD);

  /* Sort the local elements using the built-in quicksort routine */
  qsort(local_elements, nlocal, sizeof(int), incOrder);

  int j;
  /* Get into the main loop of the odd-even sorting algorithm */
  for (i = 0; i < npes - 1; i++) {
    if (i % 2 == 1) {
      MPI_Sendrecv(local_elements, nlocal, MPI_INT, oddrank, 1, recv_elements,
        nlocal, MPI_INT, oddrank, 1, MPI_COMM_WORLD, &status);
    } else {
      MPI_Sendrecv(local_elements, nlocal, MPI_INT, evenrank, 1, recv_elements,
        nlocal, MPI_INT, evenrank, 1, MPI_COMM_WORLD, &status);
    }

    int all_same = 1;
    if ((i % 2 == 1 && oddrank != MPI_PROC_NULL) || (i % 2 == 0 && evenrank != MPI_PROC_NULL)) {
      all_same = compareSplit(nlocal, local_elements, recv_elements, workspace,
        myrank < status.MPI_SOURCE);
    }

    // Using ALLgather to send local array status to all processes.
    MPI_Allgather(&all_same, 1, MPI_INT, p_same_array, 1, MPI_INT, MPI_COMM_WORLD);
    int all_p_not_changed = 1;
    for (j = 0; j < npes; j++) {
      if (p_same_array[j] != 1) {
        all_p_not_changed = 0;
        break;
      }
    }
    if (all_p_not_changed == 1) {
      break;
    }
  }

  MPI_Gather(local_elements, nlocal, MPI_INT, elements, nlocal, MPI_INT, 0, MPI_COMM_WORLD);

  free(local_elements);
  free(recv_elements);
  free(workspace);
  if (myrank == 0) {
    // printf("all sorted elms ");
    // for (i = 0; i < size; i++) {
    //   printf("%d ", elements[i]);
    // }
    // printf("\n");

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    printf ("Total time = %f seconds\n",
      (end.tv_nsec - begin.tv_nsec) / 1000000000.0 +
      (end.tv_sec  - begin.tv_sec));

    write_file(elements, size);
    free(elements);
  }
  MPI_Finalize();
}

int compareSplit(int nlocal, int *elmnts, int *relmnts, int *wspace, int keepsmall) {
  int i, j, k;
  int all_same = 1;

  for (i=0; i<nlocal; i++) {
    wspace[i] = elmnts[i]; /* Copy the elmnts array into the wspace array */
  }

  if (keepsmall) { /* Keep the nlocal smaller elements */
    for (i=j=k=0; k<nlocal; k++) {
      if (wspace[i] < relmnts[j]) {
        // check if local elements are changed or not.
        if (all_same == 1 && elmnts[k] != wspace[i]) {
          all_same = 0;
        }
        elmnts[k] = wspace[i];
        i++;
      } else {
        if (all_same == 1 && elmnts[k] != relmnts[j]) {
          all_same = 0;
        }
        elmnts[k] = relmnts[j];
        j++;
      }
    }
  }
  else { /* Keep the nlocal larger elements */ 
    for (i=j=k=nlocal-1; k >= 0; k--) {
      if (wspace[i] >= relmnts[j]) {
        if (all_same == 1 && elmnts[k] != wspace[i]) {
          all_same = 0;
        }
        elmnts[k] = wspace[i];
        i--;
      } else {
        if (all_same == 1 && elmnts[k] != relmnts[j]) {
          all_same = 0;
        }
        elmnts[k] = relmnts[j];
        j--;
      }
    }
  }

  return all_same;
}

void write_file(int *elements, int size) {
  FILE *fp1;

  fp1 = fopen("./result_modified_sort.txt", "w+");

  int i;
  for(i = 0; i < size; i++) {
    fprintf(fp1, "%d ", elements[i]);
  }

  fclose(fp1);
}

// https://stackoverflow.com/a/6852396
// Assumes 0 <= max <= RAND_MAX
// Returns in the closed interval [0, max]
int random_at_most(int max) {
  // long max = (long) max_val;
  unsigned int
    // max <= RAND_MAX < ULONG_MAX, so this is okay.
    num_bins = (unsigned int) max + 1,
    num_rand = (unsigned int) RAND_MAX + 1,
    bin_size = num_rand / num_bins,
    defect   = num_rand % num_bins;

  int x;
  do {
   x = random();
  }
  // This is carefully written not to overflow
  while (num_rand - defect <= (unsigned int)x);

  // Truncated division is intentional
  return x/bin_size;
}

int incOrder(const void *e1, const void *e2) {
  return (*((int *)e1) - *((int *)e2));
}
