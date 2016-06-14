/* This is free software. See COPYING for details. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "diag.h"
#include "debug.h"
#include "dataset.h"
#include "rotate.h"

/*** rot_dump() -- dump a rotation matrix in human-readable form

Writes an ASCII-graphic representation of a rotation matrix to the
diagnostic output in (hopefully) human-readable form.

The first argument is a pointer to a rotation_t data structure representing
the matrix in question.

The second argument is a pointer to a nul-terminated string containing
a text label for the matrix, or NULL if no label is desired.

Returns 0 on success, non-0 on error.
***/
int rot_dump(const rotation_t * const rot, const char * const lbl) {
   int len=lbl?strlen(lbl):0;

   diag(" %*s   |% 4.3lf % 4.3lf % 4.3lf|", len, "", rot->r[0][0],
    rot->r[1][0], rot->r[2][0]);
   diag("R%s = |% 4.3lf % 4.3lf % 4.3lf|", lbl?lbl:"", rot->r[0][1],
    rot->r[1][1], rot->r[2][1]);
   diag(" %*s   |% 4.3lf % 4.3lf % 4.3lf|", len, "", rot->r[0][2],
    rot->r[1][2], rot->r[2][2]);
   return 0;
}

/*** rot_find_Rz() -- find rotation about Z that puts point above/below +Y axis

Finds a matrix of rotation about the Z axis such that a specified point
will be placed somewhere directly above or below the positive Y axis. (In
other words, the {X,Y,Z} coordinates of the point after rotation will be
{0, >0, Z}.

The first argument is a pointer to a pos_t representing the point in question,
which must not be on (nor very near) the Z axis.

The second argument is a pointer to a rotation_t structure into which the
result will be written.
***/
int rot_find_Rz(const pos_t * const pos, rotation_t * const rot) {
   double theta;
   int i, j;

   theta = -atan(-pos->x / pos->y);
   if(pos->x*sin(theta) + pos->y*cos(theta) < 0) theta += M_PI;

   for(i = 0; i < 3; i++) for(j = 0; j < 3; j++) rot->r[i][j] = 0.0;
   rot->r[2][2] = 1.0;
   rot->r[0][0] = cos(theta); rot->r[1][1]=rot->r[0][0];
   rot->r[0][1] = sin(theta); rot->r[1][0]=-rot->r[0][1];
   return 0;
}

/*** rot_find_Rx() -- find rotation about X that puts a point on +Y axis

Finds a matrix of rotation about the X axis such that a specified point
will be placed directly on the positive Y axis. (In other words, the {X,Y,Z}
coordinates of the point after rotation will be {0, >0, 0}.)

The first argument is a pointer to a pos_t representing the point in question,
which must be directly above, below or on the +Y axis.

The second argument is a pointer to a rotation_t structure into which the
result will be written.
***/
int rot_find_Rx(const pos_t * const pos, rotation_t * const rot) {
   double theta;
   int i, j;

   theta = -atan(pos->z / pos->y);
   if(pos->y*cos(theta) - pos->z*sin(theta) < 0) theta += M_PI;

   for(i = 0; i < 3; i++) for(j = 0; j < 3; j++) rot->r[i][j] = 0.0;
   rot->r[0][0] = 1.0;
   rot->r[1][1] = cos(theta); rot->r[2][2]=rot->r[1][1];
   rot->r[1][2] = sin(theta); rot->r[2][1]=-rot->r[1][2];
   return 0;
}

/*** rot_find_Ry() -- find rotation about Y that puts a point on XY plane

Finds a matrix of rotation about the Y axis such that a specified point
will be placed somewhere on the -X half of the XY plane. (In other words,
the {X,Y,Z} coordinates of the point after rotation will be {<0, Y, 0}.)

The first argument is a pointer to a pos_t representing the point in question.

The second argument is a pointer to a rotation_t structure into which the
result will be written.
***/
int rot_find_Ry(const pos_t * const pos, rotation_t * const rot) {
   double theta;
   int i, j;

   theta = -atan(-pos->z / pos->x);
   if(pos->x*cos(theta) + pos->z*sin(theta) > 0) theta += M_PI;

   for(i = 0; i < 3; i++) for(j = 0; j < 3; j++) rot->r[i][j] = 0.0;
   rot->r[1][1] = 1.0;
   rot->r[0][0] = cos(theta); rot->r[2][2]=rot->r[0][0];
   rot->r[2][0] = sin(theta); rot->r[0][2]=-rot->r[2][0];
   return 0;
}

/*** rot_posn() -- rotate a single point using a rotation matrix

Rotates a point (specified by the pos_t pointed to by the second argument)
using a rotation matrix (pointed to by the third argument).

The first argument is ignored.

This function is suitable for use as a callback from dataset_foreach(),
or it can be used on its own with individual points.

The point is rotated in-place -- the result is placed back in the pos_t
pointed to by the second argument, overwriting the original data.

Returns 0 on success, non-0 on error.
***/
int rot_posn(const int i, pos_t * const pos, void * const userdata) {
   const rotation_t *rot = (const rotation_t *)userdata;
   double x, y, z;

   x=rot->r[0][0] * pos->x + rot->r[1][0] * pos->y + rot->r[2][0] * pos->z;
   y=rot->r[0][1] * pos->x + rot->r[1][1] * pos->y + rot->r[2][1] * pos->z;
   z=rot->r[0][2] * pos->x + rot->r[1][2] * pos->y + rot->r[2][2] * pos->z;

   pos->x=x; pos->y=y; pos->z=z;
   return 0;
}

/*** rot_set() -- rotate a data set using a rotation matrix

Rotates a set of points according to a rotation matrix.

The first argument is a pointer to a dynlist_t of pos_t elements.

The second argument is a pointer to a rotation_t holding the rotation
matrix.

Returns 0 on success, non-0 on error.
***/
int rot_set(dynlist_t * const lst, const rotation_t * const p) {
   return dataset_foreach((dynlist_t *)lst, rot_posn, (void *)p);
}
