#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <libspe2.h>
#include <math.h>

#include "tsplib.h"
#include "common/as.h"

extern spe_program_handle_t as_spu;

#define MAX_SPE_THREADS 16

typedef struct _ppu_thread_data {
  spe_context_ptr_t spe_context;
  void * control_block_ptr;
} ppu_thread_data_t;

void *ppu_pthread_function(void * argp)
{
  ppu_thread_data_t * thread_data;
  spe_context_ptr_t ctx;
  unsigned int entry;

  entry = SPE_DEFAULT_ENTRY;
  thread_data = (ppu_thread_data_t *) argp;
  ctx = thread_data->spe_context;

  spe_context_run(ctx, &entry, 0, thread_data->control_block_ptr, NULL, NULL);

  pthread_exit(NULL);
}

int main(int argc, char ** argv)
{
  int ** distances;
  int n, aligned_n, start_city;
  int i, j, spe_threads;
  short alpha, beta;

  pthread_t threads[MAX_SPE_THREADS];
  control_block_t * control_blocks[MAX_SPE_THREADS];
  spe_context_ptr_t spe_contexts[MAX_SPE_THREADS];
  ppu_thread_data_t ppu_thread_data[MAX_SPE_THREADS];
  unsigned int ** pheromones;

  if (argc < 4)
  {
    fprintf(stderr, "Usage: %s <tsp_file> <start_city> <alpha> <beta>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  printf("about to read tsp file!\n");
  n = parse_tsp_file(argv[1], &distances);
  aligned_n = ceil(n / 4.0) * 4;
  printf("read %d cities and allocating %d\n", n, aligned_n);
  start_city = atoi(argv[2]);
  alpha = atoi(argv[3]);
  beta  = atoi(argv[4]);

  spe_threads = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, -1);
  if (spe_threads > MAX_SPE_THREADS)
  {
    spe_threads = MAX_SPE_THREADS;
  }

  printf("will use %d spe thread(s)\n", spe_threads);

  for (i = 0; i < spe_threads; i++)
  {
    if (posix_memalign(&control_blocks[i], 128, sizeof(control_block_t)) != 0)
    {
      fprintf(stderr, "could not allocate memory for control_blocks!");
      exit(EXIT_FAILURE);
    }
    if ((spe_contexts[i] = spe_context_create(0, NULL)) == NULL)
    {
      perror("while creating spe context");
      exit(EXIT_FAILURE);
    }
    if (spe_program_load(spe_contexts[i], &as_spu) != 0)
    {
      perror("loading program into spe");
      exit(EXIT_FAILURE);
    }

    control_blocks[i]->start_city = start_city;
    control_blocks[i]->city_count = n;
    if (posix_memalign(&pheromones, 128, aligned_n * sizeof(unsigned int *)) != 0)
    {
      perror("allocating memory for pheromones matrix");
      exit(EXIT_FAILURE);
    }

    for (j = 0; j < aligned_n; j++)
    {
      posix_memalign(&pheromones[j], 128, aligned_n * sizeof(unsigned int));
    }

    control_blocks[i]->pheromones = (unsigned long long) pheromones[i];
    control_blocks[i]->distances = (unsigned long long) distances;
    control_blocks[i]->alpha = alpha;
    control_blocks[i]->beta  = beta;

    ppu_thread_data[i].spe_context = spe_contexts[i];
    ppu_thread_data[i].control_block_ptr = control_blocks[i];

    printf("distances: %llx\n", distances);

    printf("distances[0]: %llx\n", distances[0]);
    printf("distances[1]: %llx\n", distances[1]);
    printf("distances[2]: %llx\n", distances[2]);
    printf("distances[3]: %llx\n", distances[3]);

    printf("done initializing for %d, creating thread!\n", i);

    pthread_create(&threads[i], NULL, &ppu_pthread_function, &ppu_thread_data[i]);
  }

  for (i = 0; i < spe_threads; i++)
  {
    pthread_join(threads[i], NULL);
  }

  return 0;
}

