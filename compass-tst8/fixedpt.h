/* Copyright (c) 2014 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#ifndef FIXEDPT_H
#define FIXEDPT_H

#include <stdint.h>

typedef int16_t fixedpt_t; /* our fixed point type uses 16 total bits */

/* Of the 16 bits we have, the high 1 bit is a sign bit, the low
   FIXEDPT_FRACBITS are the fractional part and middle (15-FIXEDPT_FRACBITS)
   are the integer part. */
/* IMPORTANT: If you change FIXEDPT_FRACBITS, you'll need to also reconsider
   the value of FIXEDPT_BRAD_SEMICIRC as well as the table of constants
   in fxp_atan2(). Also be careful of scaling, overflow and magic constants
   in the calling program! */
#define FIXEDPT_FRACBITS 11

#define FIXEDPT_ZERO 0
#define FIXEDPT_ONE (1 << FIXEDPT_FRACBITS)

/* Our trig functions will use binary radians, where half a circle is
   FIXEDPT_BRAD_SEMICIRC. */
#define FIXEDPT_BRAD_SEMICIRC (FIXEDPT_ONE * 8)

/* You can add, subtract and shift fixed-point values like you would
for any other integer. (Just be careful of overflow.) You can also multiply
and divide by (but not add/subtract) normal integers.

To multiply or divide two fixed-point values, use fxp_mul() or fxp_div().
Note that these involve a real 32-bit signed multiply or divide (respectively),
so should be considered moderately expensive. */

fixedpt_t fxp_mul(const fixedpt_t a, const fixedpt_t b);
fixedpt_t fxp_div(const fixedpt_t a, const fixedpt_t b);

fixedpt_t fxp_atan2(fixedpt_t x, fixedpt_t y);
fixedpt_t fxp_dist(const fixedpt_t a, const fixedpt_t b, const fixedpt_t c);
fixedpt_t fxp_abs(const fixedpt_t x);

#define fxp(x) (truncf((x)*(1 << FIXEDPT_FRACBITS)))
#define flt(x) (((float)(x)) / (1 << FIXEDPT_FRACBITS))

#endif /* ifndef FIXEDPT_H */
