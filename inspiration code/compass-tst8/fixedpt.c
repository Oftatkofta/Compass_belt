/* Copyright (c) 2014 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#include <stdint.h>
#include "fixedpt.h"

typedef int32_t fxp_t;

/*** fxp_mul() -- multiply two fixed-point values

Multiples the two fixed-point values represented by the arguments, and
returns the product. Note that the result is truncated rather than rounded,
and that no overflow detection is done.
***/
fixedpt_t fxp_mul(const fixedpt_t a, const fixedpt_t b) {
   return (((fxp_t)a) * ((fxp_t)b)) >> FIXEDPT_FRACBITS;
}

/*** fxp_div() -- divide one fixed-point value by another

Divides the fixed-point value represented by the first argument by that
represented by the second. Returns the result. Note that the result is
truncated rather than rounded, and there is no overflow detection nor
protection against division by zero.
***/
fixedpt_t fxp_div(const fixedpt_t a, const fixedpt_t b) {
   return ((((fxp_t)a) << 16) / ((fxp_t)b)) >> (16-FIXEDPT_FRACBITS);
}

/* Table of coefficients used for CORDIC atan2() implementation. Values
   are atan(2^(-i)) in binary radians (where a semicircle is
   FIXEDPT_BRAD_SEMICIRC). Note that _tbl[0] is never used, but shifting
   all the elements costs more than two bytes elsewhere... */
// FIXME: in AVR version, put this in PROGMEM
static const fixedpt_t _tbl[] = {
   0x4000, 0x25c8, 0x13f6, 0x0a22, 0x0516, 0x028c,
   0x0146, 0x00a3, 0x0051, 0x0029, 0x0014, 0x000a,
   // 0x0005, 0x0003, 0x0001, 0x0001
};

/*** fxp_atan2() -- compute inverse tangent in binary radians

Finds the angle (in binary radians, where FIXEDPT_BRAD_SEMICIRC is a half-
circle) from the positive X axis to the point defined by {x,y}.

The arguments are the y and x coordinates, *in that order*, as fixed-point
values.

Returns a fixed-point value representing atan(y/x) (in the correct octant).
***/
fixedpt_t fxp_atan2(fixedpt_t y, fixedpt_t x) {
    fixedpt_t phi=0, i, tmp, dphi=0;

    /* Implementation adapted from fixed-point CORDIC described here:
          http://www.coranac.com/documents/arctangent/
       by Jasper Vijn <cearn@coranac.com> */

    if(y==0) return x>=0 ? 0 : FIXEDPT_BRAD_SEMICIRC; /* edge case */

    /* Get {x,y} into the first octant, and modify our result accordingly: */
    if(y<0)  {          x=-x; y=-y;   phi += FIXEDPT_BRAD_SEMICIRC; }
    if(x<=0) { tmp=x;   x=y;  y=-tmp; phi += FIXEDPT_BRAD_SEMICIRC/2; }
    if(x<=y) { tmp=y-x; x+=y; y=tmp;  phi += FIXEDPT_BRAD_SEMICIRC/4; }

    // FIXME: scale up for greater accuracy if both x and y are small?
    // (Probably not useful for compass as {x,y} pairs are all near a circle
    // centered on the origin.)

    /* Perform the CORDIC approximation */
    for(i=1; i<12; i++) {
        if(y>=0) {
            tmp= x + (y>>i);
            y  = y - (x>>i);
            x  = tmp;
            dphi += _tbl[i];
        } else {
            tmp= x - (y>>i);
            y  = y + (x>>i);
            x  = tmp;
            dphi -= _tbl[i];
        }
    }

    return phi + (dphi>>2);
}

/*** fxp_dist() -- find distance from origin to point in three-space

Given the cartesian coordinates of a point in three-space (X, Y and Z
in arbitrary order), this function returns the distance from the origin
to the point in question.

The arguments are the coordinates as fixedpt_t numbers.

Returns the result as a fixedpt_t.
***/
fixedpt_t fxp_dist(const fixedpt_t a, const fixedpt_t b, const fixedpt_t c) {
    fxp_t x, res=0, bit;

    /* let x be the square of the distance (as a fixed-point 32-bit number
       with FIXEDPT_FRACBITS*2 bits to the right of the binary point) */
    x = (fxp_t)a * (fxp_t)a + (fxp_t)b * (fxp_t)b + (fxp_t)c * (fxp_t)c;

    /* let bit be the highest power of four <= x */
    for(bit = 1L<<30; bit > x; bit >>= 2);

    while(bit) {
        if(x >= res + bit) {
            x -= res + bit;
            res = (res >> 1) + bit;
        } else res >>= 1;
        bit >>= 2;
    }
    return res;
}

fixedpt_t fxp_abs(const fixedpt_t x) { return x > 0 ? x : -x; }
