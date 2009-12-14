#ifndef __TSP_H_
#define __TSP_H_

typedef struct _control_block {
  unsigned long long address;
  int size;
  int partitions;
  int current;
  int dummy;
} control_block;

#endif /* __TSP_H_ */

