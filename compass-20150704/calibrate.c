/* Copyright (c) 2014-2015 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#include <string.h>
#include <avr/pgmspace.h>
#include <hmc5883l/hmc5883l.h>
#include <matrix8x8/matrix8x8.h>
#include "rotate.h"
#include "calibrate.h"
#include "button.h"

typedef struct { hmc5883l_pos_t n, w, c; } nwc_t;

#if FIXEDPT_FRACBITS < 6
#error This code assumes FIXEDPT_FRACBITS is at least 6.
#endif
#define DELTA ((fixedpt_t)(1 << (FIXEDPT_FRACBITS-6)))

#define DIST(a,b) fxp_dist(a.x - b.x, a.y - b.y, a.z - b.z)

/*** get_nwc() -- obtain points N, W and C used as basis for calibration

This function obtains the points N, W and C used as the basis for creating
the calibration matrix.

Point N is the sensor reading when the vehicle is facing (geographic)
North.

Point C is the center of the ellipse created by the sensor readings when
the vehicle undergoes a complete rotation about the yaw axis.

Point W is 1) somewhere on the aforementioned ellipse, 2) not colinear with
N and S and 3) somewhere on the Western half of the compass rose.

The argument is a pointer to an nwc_t structure which will be
populated with the results.

Returns:
   0 -- success
  <0 -- one of the HMC5883L_ERR_* values (other than HMC5883L_ERR_OK)
      indicating a failure due to a magnetic sensor problem
  >0 -- user cancelled operation
***/
static int get_nwc(nwc_t * const p) {
   fixedpt_t dist_N, dist_NS, dist_S, diff, best;
   int32_t ctr_x = 0L, ctr_y = 0L, ctr_z = 0L;
   hmc5883l_pos_t pos_S, pos_prev, pos;
   int n, rtn;

   /* Set N, S and pos_prev to the starting position: */
   if((rtn = hmc5883l_read(&(p->n))) != HMC5883L_ERR_OK) return rtn;
   poscp(pos_S, p->n);
   poscp(pos_prev, p->n);
   n = 0;
   dist_NS = FIXEDPT_ZERO; /* N and S are the same, so distance is zero */
   best = FIXEDPT_ZERO;

   do { /* calibration loop */
      if(button()) return 1; /* button pressed -> cancel calibration */

      /* take a reading */
      if((rtn = hmc5883l_read(&(pos))) != HMC5883L_ERR_OK) return rtn;

      /* Find the distances from the point just read to the current N and S: */
      dist_N = DIST(pos, p->n);
      dist_S = DIST(pos, pos_S);

      /* If the current point is more than a fixed distance away from
         pos_prev, add it to the points averaged to find the center and
         let it be the new pos_prev: */
      if(DIST(pos, pos_prev) > DELTA) {
         ctr_x+=pos.x; ctr_y+=pos.y; ctr_z+=pos.z;
         n++;
         poscp(pos_prev, pos);
      }

      /* If the point just read is further away from N that the current S
         by more than a tiny amount (compared to the distance from the
         current point to S), then the current point is our new S.

         The complexity is required because the points are on an elliptic
         curve. If N is near the minor axis of a highly-eccentric ellipse,
         then there are two points at a maximum distance from N, and we
         want to find the first one. Because of jitter, the second one may
         look (slightly) further away... */
      if(dist_N - dist_NS > (dist_S >> 3)) {
         poscp(pos_S, pos);
         best = dist_NS = dist_N; dist_S = FIXEDPT_ZERO;
      }

      diff = fxp_abs(dist_N - dist_S);
      if(diff < best) {
         best = diff;
         poscp(p->w, pos);
      }

   /* End the calibration loop when the most recent reading is close to
      the original position and the distance from N to S is big. */
   } while((dist_N >> 1) > DELTA || (dist_NS >> 2) < DELTA);

   p->c.x = ctr_x / n;
   p->c.y = ctr_y / n;
   p->c.z = ctr_z / n;

   return 0;
}

/*** elem_rot() -- find and perform an elemental rotation

Finds and performs an elemental rotation, rotating the N and W points
and (optionally) multiplying an existing rotation matrix.

The first four arguments are as per rot_find(), which see.

Argument five is a pointer to an nwc_t. The N and W points inside will
be transformed using the rotation found.

Argument six is a pointer to a rotation matrix to be multiplied by the
new rotation, or NULL if no such multiplication is needed.
***/
static void elem_rot(const fixedpt_t a, const fixedpt_t b, const int idx,
 rotation_t * const dst, nwc_t * const nwc, rotation_t * const prod) {
   rot_find(a, b, idx, dst);
   rot_posn(&(nwc->n), dst);
   rot_posn(&(nwc->w), dst);
   if(prod) rot_mult(prod, dst);
}

/*** calibrate() -- find translation and rotation data for magnetometer

Performs a calibration process to find a translation vector and rotation
matrix such that any magnetometer reading can be translated and rotated
so that it will lie on a circle in the XY plane, centered at the origin,
with North on the positive Y axis and West on the negative X axis.

The argument is a pointer to a calibration_t vector to be populated
with the calibration data.

Returns 0 on success, non-0 on error.

Note that the calibration process assumes:

1) The user will begin the process facing exactly geographic North.

2) The user will turn in a full circle to the right. (To be specific: The
   user will make a full rotation about the yaw axis. The path need not be
   a circle, and neither the speed nor the turn rate need be constant.)

Returns:
   0 -- success
  <0 -- one of the HMC5883L_ERR_* values (other than HMC5883L_ERR_OK)
      indicating a failure due to a magnetic sensor problem
  >0 -- user cancelled operation
***/
int calibrate(calibration_t * const p) {
   rotation_t r;
   nwc_t nwc;
   uint8_t img[8];
   int rtn;
 
   /* perform self-test and get scaling data for temperature compensation */
   if((rtn = hmc5883l_test(&(p->scale))) != HMC5883L_ERR_OK) return rtn;

   /* "FACE NORTH" */ 
   memset(img, 0, 8);
   img[1] = 0b00001111; img[3] = 0b01111100;
   matrix8x8_draw(img);

   while(!button()); /* wait for button press */

   /* "TURN CW" */
   memset(img, 0, 8); img[0] = 0b00001111;
   img[1] = 0b11110000; img[5] = 0b10000000; img[6] = 0b10000000;
   matrix8x8_draw(img);
   
   if((rtn = get_nwc(&nwc)) != HMC5883L_ERR_OK) return rtn;

   /* We know the translation immediately; it's just the center point. */
   poscp(p->xlate, nwc.c);

   /* Translate N and W so that C is the new origin: */
   nwc.n.x -= nwc.c.x; nwc.n.y -= nwc.c.y; nwc.n.z -= nwc.c.z;
   nwc.w.x -= nwc.c.x; nwc.w.y -= nwc.c.y; nwc.w.z -= nwc.c.z;

   /* If N is closer to the Y axis than to the Z axis... */
   if(fxp_abs(nwc.n.x) > fxp_abs(nwc.n.z)) {
      /* Find Rz to put N above/below the positive Y axis on the YZ plane: */
      elem_rot(-(nwc.n.x), nwc.n.y, 2, &(p->rotate), &nwc, NULL);

      /* Find Rx to put N on the positive Y axis: */
      elem_rot(nwc.n.z, nwc.n.y, 0, &r, &nwc, &(p->rotate));

   } else { /* N is closer to the Z axis than the X axis */
      /* Find Rx to put N to the left/right of the positive Y axis on the
         XY plane: */
      elem_rot(nwc.n.z, nwc.n.y, 0, &(p->rotate), &nwc, NULL);

      /* Find Rz to put N on the positive Y axis: */
      elem_rot(-(nwc.n.x), nwc.n.y, 2, &r, &nwc, &(p->rotate));
   }

   /* Find Ry to put W on the negative-X half of the XY plane. */
   elem_rot(nwc.w.z, -(nwc.w.x), 1, &r, &nwc, &(p->rotate));

   return 0;
}
