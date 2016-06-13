#include <stdint.h>
#include <hmc5883l/hmc5883l.h>
#include "validate.h"

/* TILT_RATIO is the sine of the angle beyond which a tilt state is
   considered to exist, as a fixedpt_t. */
/* 2^FIXEDPT_FRACBITS=2^11=2048; sin(20)*2048 ~= 0x02BC */
#define TILT_RATIO ((fixedpt_t)0x02BC) /* ~20 degrees */

/* MAG_MIN and MAG_MAX give the (inclusive) range of absolute magnitude
   of magnetometer reading (after correction for hard-iron interference)
   considered to be reasonable for Earth's magnetic field. Readings with
   magnitudes outside this range are considered to be artifacts of an
   interfering field.

   Per Wikipedia, the approximate magnitude of Earth's field at the surface
   is approximately 0.25 to 0.65 Gauss. Since we can compute meaningful
   headings even in the face of some interference, we will permit a somewhat
   wider range: 0.20 to 0.80 Gauss.

   The magnetometer gain is set to 1090 LSb/Ga, so the components of the
   reading (and the overall magnitude) can be treated as integers with units
   of (1/1090)Ga.

   Thus, the limits are 0.20*1090 = 0x00DA, 0.80*1090 = 0x0368 */
#define MAG_MIN ((fixedpt_t)0x00DA)
#define MAG_MAX ((fixedpt_t)0x0368)

/*** validate() -- test if adjusted magnetometer reading makes sense

Performs validation on an adjusted magnetometer reading. ("Adjusted" means
after the constant offset has been subtracted away, and after the reading
has been rotated using the calibration matrix.)

Arguments:
   pos -- pointer to hmc5883l_pos_t containing adjusted reading

Returns a binary-or of zero or more of the following:
   VLD_ERR_TILT -- adjusted reading is too far from the plane of rotation
      established during calibration
   VLD_ERR_INTF -- the absolute magnitude of the adjusted reading is too
      large or too small relative to the expected strength of the Earth's
      magnetic field at the surface

If no problem is detected, returns 0 (VLD_ERR_OK).
***/
int validate(const hmc5883l_pos_t * const pos) {
   int rtn = VLD_ERR_OK;
   fixedpt_t dist;

   /* Let dist be the distance from the reading to the origin: */
   dist = fxp_dist(pos->x, pos->y, pos->z);

   /* Imagine a line segment S from the origin to the reading. If
      the angle theta between S and the XY plane exceeds a threshold, then
      a tilt condition exists. Since sin(theta) = pos->z / dist, we can
      test for tilt simply by checking if (pos->z / dist) is greater than
      a constant. */
   if(fxp_abs(fxp_div(pos->z, dist)) > TILT_RATIO) rtn |= VLD_ERR_TILT;

   /* If the absolute magnitude of the reading isn't in a range that makes
      sense for Earth's natural magnetic field, return an interference
      indication. */
   if(dist < MAG_MIN || dist > MAG_MAX) rtn |= VLD_ERR_INTF;

   return rtn;
}
