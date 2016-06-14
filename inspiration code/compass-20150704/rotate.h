/* Copyright (c) 2014-2015 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#ifndef ROTATE_H
#define ROTATE_H

#include <hmc5883l/hmc5883l.h>
#include "fixedpt.h"

typedef struct { fixedpt_t r[3][3]; } rotation_t;

void rot_find(const fixedpt_t a, const fixedpt_t b, const int idx,
 rotation_t * const rot);

void rot_posn(hmc5883l_pos_t * const pos, const rotation_t * const rot);

void rot_mult(rotation_t * const a, const rotation_t * const b);

/*** poscp() -- copy one point to another

This macro is used to copy the contents of one hmc5883l_pos_t into another.
It saves a little bit of code space over using memcpy()(3).

The first argument is the structure into which data is being copied. The
second argument is the structure from which the data is being copied. Note
that neither argument is a pointer!
***/
#define poscp(dst,src) do { (dst).x=(src).x; (dst).y=(src).y; \
 (dst).z=(src).z; } while(0)

#endif /* ifndef ROTATE_H */
