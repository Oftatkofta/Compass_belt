/* Copyright (c) 2014-2015 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fixedpt.h"
#include "rotate.h"

/*** rot_find() -- find elemental rotation puts point on given plane

Finds a matrix of rotation about the Z axis such that a specified point
will be placed somewhere directly above or below the positive Y axis. (In
other words, the {X,Y,Z} coordinates of the point after rotation will be
{0, >0, Z}.

The first and second arguments are coordinates for the two axes which
are orthogonal to the axis of rotation. The rotation will be chosen such
that the coordinate associated with the first argument becomes zero in the
rotated system.

The third argument specifies which elemental rotation is to be performed:
   0 = Rx
   1 = Ry
   2 = Rz

The fourth argument is a pointer to a rotation_t structure into which the
result will be written.
***/
void rot_find(const fixedpt_t a, const fixedpt_t b, const int idx,
 rotation_t * const rot) {
   fixedpt_t d, dsin, dcos;
   int i, j;

   d = fxp_dist(a, b, FIXEDPT_ZERO); /* equiv. to d = sqrtf(a*a + b*b) */
   dsin = -fxp_div(a,d); dcos = fxp_div(b,d);

   for(i = 0; i < 3; i++) for(j = 0; j < 3; j++) rot->r[i][j] = FIXEDPT_ZERO;
   i = idx+1; if(i == 3) i = 0;
   j = i+1; if(j == 3) j = 0;
   rot->r[idx][idx] = FIXEDPT_ONE;
   rot->r[i][i] = rot->r[j][j] = dcos;
   rot->r[i][j] = dsin; rot->r[j][i] = -dsin;
}

/*** rot_posn() -- rotate a single point using a rotation matrix

Rotates a point (specified by the hmc5883l_pos_t pointed to by the first
argument) using a rotation matrix (pointed to by the second argument).

The point is rotated in-place -- the result is placed back in the pos_t
pointed to by the second argument, overwriting the original data.
***/
void rot_posn(hmc5883l_pos_t * const pos, const rotation_t * const rot) {
   fixedpt_t x, y, z;

   x=fxp_mul(rot->r[0][0], pos->x) +
     fxp_mul(rot->r[1][0], pos->y) +
     fxp_mul(rot->r[2][0], pos->z);

   y=fxp_mul(rot->r[0][1], pos->x) +
     fxp_mul(rot->r[1][1], pos->y) +
     fxp_mul(rot->r[2][1], pos->z);

   z=fxp_mul(rot->r[0][2], pos->x) +
     fxp_mul(rot->r[1][2], pos->y) +
     fxp_mul(rot->r[2][2], pos->z);

   pos->x=x; pos->y=y; pos->z=z;
}

/*** rot_mult() -- multiply rotation matrix A by B, leaving result in A

Performs the matrix multiplication AB (where A and B are 3x3 elemental
rotation matrices), then copies the result into A, overwriting the original
contents.

The arguments are pointers to the rotation matrices in question.
***/
void rot_mult(rotation_t * const a, const rotation_t * const b) {
   rotation_t r;
   int i, j;

   for(i = 0; i < 3; i++) {
      for(j = 0; j < 3; j++) {
         r.r[i][j] = fxp_mul(a->r[i][0], b->r[0][j]) +
                     fxp_mul(a->r[i][1], b->r[1][j]) +
                     fxp_mul(a->r[i][2], b->r[2][j]);
      }
   }

   memcpy(a, &r, sizeof(r));
}
