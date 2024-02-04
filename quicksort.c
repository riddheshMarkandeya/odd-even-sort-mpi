#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_VALUE 128

void write_file(int *elements, int size);
int random_at_most(int max);
int incOrder(const void *e1, const void *e2);


int main(int argc, char* argv[]) {
  srand(time(0));

  int *elements;
  int size;
  int i;
  struct timespec begin, end;

  if (argc == 1) {
    printf("No argument provided for array size exponent taking 128 as default.\n");
    printf("e.g. Run \"./a.out 10\" for size 2^10.\n");
    size = 128;
  } else {
    size = (int)pow(2, (double)atoi(argv[1]));
    // printf("%d: Array size is %d %d\n", myrank, size, npes);
  }

  elements = (int *)malloc(size * sizeof(int));
  for(i = 0; i < size; i++) {
    elements[i] = random_at_most(MAX_VALUE);
  }

  clock_gettime(CLOCK_MONOTONIC_RAW, &begin);

  qsort(elements, size, sizeof(int), incOrder);

  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  printf ("Total time = %f seconds\n",
    (end.tv_nsec - begin.tv_nsec) / 1000000000.0 +
    (end.tv_sec  - begin.tv_sec));

  write_file(elements, size);
  free(elements);
}

void write_file(int *elements, int size) {
  FILE *fp1;

  fp1 = fopen("./result_quicksort.txt", "w+");

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
