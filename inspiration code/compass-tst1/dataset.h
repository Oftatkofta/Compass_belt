#ifndef DATASET_H
#define DATASET_H

#include "dynlist.h"

typedef struct { double x,y,z; } pos_t;

dynlist_t *dataset_read(const char * const fname);

int dataset_foreach(dynlist_t * const lst, int(*hdl)(const int i,
 pos_t * const pos, void * const userdata), void * const userdata);

int dataset_dump(const dynlist_t * const lst);

#endif /* ifndef DATASET_H */
