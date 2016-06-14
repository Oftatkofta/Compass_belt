#ifndef ROTATE_H
#define ROTATE_H

#include "dataset.h"
#include "dynlist.h"

typedef struct { double r[3][3]; } rotation_t;

int rot_dump(const rotation_t * const rot, const char * const lbl);

int rot_find_Rz(const pos_t * const pos, rotation_t * const rot);
int rot_find_Rx(const pos_t * const pos, rotation_t * const rot);
int rot_find_Ry(const pos_t * const pos, rotation_t * const rot);

int rot_posn(const int i, pos_t * const pos, void * const userdata);

int rot_set(dynlist_t * const lst, const rotation_t * const p);

#endif /* ifndef ROTATE_H */
