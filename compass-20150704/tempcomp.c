#include <hmc5883l/hmc5883l.h>
#include "tempcomp.h"

/*** tempcomp() -- perform temperature compensation of magnetometer reading

Scales raw magnetometer data to compensate for variations in sensor
sensitivity due to temperature.

The AMR sensors in the magnetometer become more or less sensitive as the
sensor temperature varies. To compensate for this, we use the self-test
feature to measure magnetic field of known strength. Specifically, we
measure the ambient field with and without a known field superimposed,
then subtract the two. Since the two measurements are very close together
in time, the ambient field is close to constant, so the difference is a
good representation of the sensor response to a field of known strength.

Given two such measurements (one from the time of calibration, and one
which is recent) we can quantify how much more or less sensitive each
sensor axis has become between calibration time and now. This, in turn,
allows us to scale a current reading to the same sensitivity level in
effect during calibration.

Arguments:
   pos -- pointer to hmc5883l_pos_t containing raw sensor reading (before
      any translation or rotation); this data will be modified in-place
      to contain the temperature-compensated equivalent.
   cur -- sensitivty measurement from self-test, acquired near the same
      time as the sample pointed to by pos
   orig -- sensitivity measurement from the time of calibration
***/
void tempcomp(hmc5883l_pos_t * const pos, const hmc5883l_pos_t * const cur,
 const hmc5883l_pos_t * const orig) {
   pos->x = fxp_scale(pos->x, orig->x, cur->x);
   pos->y = fxp_scale(pos->y, orig->y, cur->y);
   pos->z = fxp_scale(pos->z, orig->z, cur->z);
}
