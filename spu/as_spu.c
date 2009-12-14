#include <stdio.h>
#include <spu_mfcio.h>
#include <math.h>

#include "common/as.h"

/**
 * Based on:
 * http://www.tk.uni-linz.ac.at/download/papers/gk_kluwer98.pdf
 */

/**
 * p function. Probabilty to go from city **i** to **j**
 *
 * @param i        - city from
 * @param j        - city to
 * @param n        - number of cities
 * @param tao      - pheromones' matrix
 * @param alpha    - adaptative memory parameter
 * @param beta     - desirability parameter
 * @param omega    - array of cities: 1 means visited, 0 means not yet visited
 * @param distance - matrix to represent distance between cities
**/
double p(int j, int n, int * tao, int alpha, int beta, char * omega, int * distance)
{
  int h;
  double num, denom;

  if (omega[j] == 1 || distance[j] == 0)
  {
    return 0;
  }
  else
  {
    num = powf(tao[j], 1.0/alpha) * powf(1.0/distance[j], 1.0/beta);
    denom = 0;

    for (h = 0; h < n; h++)
    {
      if (omega[h] == 0)
      {
        denom += powf(tao[h], 1.0/alpha) * powf(1.0/distance[h], 1.0/beta);
      }
    }

    return num/denom;
  }
}

void run_problem(int alpha, int beta, int city_count, int start_city, unsigned long long pheromones, unsigned long long distances)
{
  int i, current_city, tag_id, aligned_city_count, max_i;
  aligned_city_count = ceil(city_count / 4.0) * 4;
  char omega[aligned_city_count];
  int current_distances[aligned_city_count];
  int current_pheromones[aligned_city_count];
  unsigned int not_visited = city_count;
  double max_p, current_p;
  unsigned long long distances_addresses[aligned_city_count];
  unsigned long long pheromones_addresses[aligned_city_count];
  unsigned long long distance_traveled;

  tag_id = mfc_tag_reserve();

  mfc_get(&distances_addresses, distances, sizeof(unsigned long long) * aligned_city_count, tag_id, 0, 0);
  mfc_get(&pheromones_addresses, distances, sizeof(unsigned long long) * aligned_city_count, tag_id, 0, 0);
  mfc_write_tag_mask(1 << tag_id);
  mfc_read_tag_status_all();

  for (i = 0; i < city_count; i++)
  {
    omega[i] = 0;
  }

  omega[start_city] = 1;
  current_city = start_city;

  distance_traveled = 0;

  while (not_visited > 0)
  {
    printf("%d -> ", current_city);
    mfc_get(current_distances, distances_addresses[current_city], sizeof(unsigned int) * aligned_city_count, tag_id, 0, 0);
    mfc_get(current_pheromones, pheromones_addresses[current_city], sizeof(unsigned int) * aligned_city_count, tag_id, 0, 0);
    mfc_write_tag_mask(1 << tag_id);
    mfc_read_tag_status_all();

    max_p = -1;
    for (i = 0; i < city_count; i++)
    {
      current_p = p(i, city_count, current_pheromones, alpha, beta, omega, current_distances);
      if (current_p >= max_p)
      {
        max_p = current_p;
        max_i = i;
      }
    }
    printf("(%lf) ", max_p);

    distance_traveled += current_distances[max_i];
    current_pheromones[max_i] += 1000;

    mfc_put(current_pheromones, pheromones_addresses[current_city], sizeof(unsigned int) * aligned_city_count, tag_id, 0, 0);
    mfc_write_tag_mask(1 << tag_id);
    mfc_read_tag_status_all();

    current_city = max_i;

    omega[current_city] = 1;
    not_visited--;

    for (i = 0; i < city_count; i++)
    {
      current_pheromones[i] -= 10;
    }
  }

  current_pheromones[start_city] += 1000;

  printf("%d\n", start_city);
  printf("distance traveled on iteration: %lld\n", distance_traveled);
}

int main(
  unsigned long long speid __attribute__ ((unused)),
  unsigned long long argp,
  unsigned long long envp __attribute__ ((unused)))
{
  control_block_t cb __attribute__ ((aligned (128)));
  unsigned int tag_id;
  int i;

  tag_id = mfc_tag_reserve();

  mfc_get(&cb, argp, sizeof(cb), tag_id, 0, 0);
  mfc_write_tag_mask(1 << tag_id);
  mfc_read_tag_status_all();

  for (i = 0; i < 4; i++)
  {
    run_problem(cb.alpha, cb.alpha, cb.city_count, cb.start_city, cb.pheromones, cb.distances);
  }

  return 0;
}

