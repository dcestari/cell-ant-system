#ifndef __AS_H_
#define __AS_H_

typedef struct _control_block {
  unsigned short city_count;
  unsigned short start_city;
  unsigned short alpha;
  unsigned short beta;
  unsigned long long pheromones;
  unsigned long long distances;
  unsigned long long out_addr;
} control_block_t;

#endif /* __AS_H_ */

