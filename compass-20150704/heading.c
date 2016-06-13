/* Copyright (c) 2014-2015 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hmc5883l/hmc5883l.h>
#include "rotate.h"
#include "calibrate.h"
#include "heading.h"

/*** heading() -- transform magnetometer reading into compass heading

Takes a reading from a three-axis magnetometer and transforms it into
a scalar compass heading using calibration data obtained previously.

The first argument is a pointer to a structure representing the raw
magnetometer reading. The contents of this structure will be shifted
and rotated according to the calibration data.

The second argument is a pointer to a structure containing the calibration
data previously obtained using calibrate().

Returns a heading (in binary radians to the right of geographic north).
Multiply by 180.0/FIXEDPT_BRAD_SEMICIRC to get degrees.
***/
fixedpt_t heading(hmc5883l_pos_t * const pos,
 const calibration_t * const calib) {
   pos->x -= calib->xlate.x; pos->y -= calib->xlate.y; pos->z -= calib->xlate.z;
   rot_posn(pos, &(calib->rotate));

   return fxp_atan2(pos->x, pos->y);
}
