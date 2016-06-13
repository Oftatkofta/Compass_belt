/* Copyright (c) 2014-2015 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#include <stdlib.h>
#include <util/delay.h>
#include <uWireM/uWireM.h>
#include "hmc5883l.h"

#define HMC5883L_I2C_ADDR 0x1e

/* Number of samples averaged per reading: */
#define CFG_AVG_1 0x00
#define CFG_AVG_2 0x01
#define CFG_AVG_4 0x02
#define CFG_AVG_8 0x03

/* Load bias direction: */
#define CFG_BIAS_NONE 0x00
#define CFG_BIAS_POS  0x01
#define CFG_BIAS_NEG  0x02

/* Gain (in LSb per Gauss): */
#define CFG_GAIN_1370 0x00
#define CFG_GAIN_1090 0x01
#define CFG_GAIN_820  0x02
#define CFG_GAIN_660  0x03
#define CFG_GAIN_440  0x04
#define CFG_GAIN_390  0x05
#define CFG_GAIN_330  0x06
#define CFG_GAIN_230  0x07

/* Configuration choices: */
#define CFG_AVG CFG_AVG_8
#define CFG_GAIN CFG_GAIN_1090

/* Register addresses on HMC5883L: */
#define REG_CRA 0  /* config A */
#define REG_CRB 1  /* config B */
#define REG_MR 2   /* mode */
#define REG_DATA 3 /* registers 3-8 inclusive are data, read as a block */
#define REG_SR 9   /* status */
#define REG_IRA 10 /* ID A = 'H' */
#define REG_IRB 11 /* ID B = '4' */
#define REG_IRC 12 /* ID C = '3' */

/* With gain=5 (i.e., CFG_GAIN_390 = 390 LSb per gauss), the range of
   acceptable selftest delta value for all three axes are given (per the
   data sheet) by the following. (For other gain values, scale the limits
   proportionally.) */
#define SELFTEST_POS_MIN ((int32_t)243)
#define SELFTEST_POS_MAX ((int32_t)575)

/* From the data sheet: "In the event the ADC reading overflows or underflows
   for the given channel, or if there is a math overflow during the bias
   measurement, [the result registers for the axis in question] will contain
   the value -4096." */
#define AXIS_SATURATED (-4096)

static uint8_t _buf[7];

/*** val() -- convert pair of result registers to twos-comp. 16-bit number

Given the contents of a pair of axis output registers (such as DXRA and DXRB
for the X axis, or the analogous Y or Z registers), calculates the axis
value.

Arguments:
   p -- pointer to register buffer such that p[0] contains the value of
      output register A and p[1] contains the value of output register B
      for the axis to be tested

Returns the axis value as a 16-bit two's complement signed number.
***/
static __attribute__((pure)) int16_t val(const uint8_t * const p) {
   int16_t x;
   /* Do not be tempted to do something like the following; it won't work!
      return *((int16_t *)p); -- WRONG
   */
   x = p[0]; x = (x << 8) | p[1];
   return x;
}

/*** hmc5883l_read() -- read sensor values from HMC5883L

Read the sensor values from the HMC5883L.

Note: Results are undefined if HMC5883L has not yet been configured
using hmc5883l_config().

Arguments:
   p -- pointer to data structure into which sensor data is to be stored

Returns one of the HMC5883L_ERR_* values. If the return is anything other
than HMC5883L_ERR_OK, the contents of the structure pointed to by p are
undefined.
***/
int hmc5883l_read(hmc5883l_pos_t * const p) {

   /* Start a measurement in single measurement mode: */
   _buf[0] = uWireM_addr_send(HMC5883L_I2C_ADDR);
   _buf[1] = REG_MR;
   _buf[2] = 0x01;
   if(!uWireM_xfer(_buf, 3)) return HMC5883L_ERR_I2C;

   /* Tell the device we want to start reading at DXRA (register 3): */
   _buf[1] = REG_DATA;
   if(!uWireM_xfer(_buf, 2)) return HMC5883L_ERR_I2C;

   /* We could poll the status register here, but to save some space,
      we'll just block for a minimum of (1Hz/s)/160Hz = 6250us, after
      which we know the reading will be ready. */
   _delay_us(6250);

   /* Read all six positions: */
   _buf[0] = uWireM_addr_recv(HMC5883L_I2C_ADDR);
   if(!uWireM_xfer(_buf, 7)) return HMC5883L_ERR_I2C;

   /* Convert raw data to numbers and put in result buffer: */
   p->x = val(_buf+1); p->y = val(_buf+3); p->z = val(_buf+5);

   /* Test for saturation along any of the axes: */
   if(p->x == AXIS_SATURATED || p->y == AXIS_SATURATED ||
    p->z == AXIS_SATURATED)
      return HMC5883L_ERR_SATURATED;

   return HMC5883L_ERR_OK;
}

/*** conf() -- configure HMC5883L sensor

Performs initialization and configuration on HMC5883L sensor.

The gain and averaging modes are set per the CFG_AVG and CFG_GAIN
defines above. The sensor is placed in idle mode, and the data output
rate bits are left all zero.

Arguments:
   bias -- bias mode; should be CFG_BIAS_NONE for normal operation,
      or one of CFG_BIAS_POS or CFG_BIAS_NEG for self-test (or
      temperature compensation)

Returns 0 on success, non-0 on failure.
***/
static int conf(const uint8_t bias) {
   /* Set config registers as follows:

      REG_CRA is set for desired averaging and bias. We leave
      the output rate all-bits-zero since we're just going to be using
      single measurement mode.

      REG_CRB is set for the desired gain.

      REG_MR is set to force idle mode.

      Note that these registers are contiguous (in the above order) so
      we can set them all with a single write.
   */
   _buf[0] = uWireM_addr_send(HMC5883L_I2C_ADDR);
   _buf[1] = REG_CRA;
   _buf[2] = CFG_AVG << 5 | bias;      /* REG_CRA */
   _buf[3] = CFG_GAIN << 5;            /* REG_CRB */
   _buf[4] = 0x03;                     /* REG_MR */

   /* Note that uWireM_xfer() has a reversed return; TRUE for success,
      FALSE for failure. This is a consequence of the Atmel-supplied
      I2C code underlying the uWireM library. */
   return !uWireM_xfer(_buf, 5);
}

/*** hmc5883l_init() -- configure HMC5883L sensor

Performs initialization and configuration on HMC5883L sensor.
Initializes sensor for normal operation. 

Returns one of the HMC5883L_ERR_* values.
***/
int hmc5883l_init(void) {
   hmc5883l_pos_t buf;

   /* configure for normal operation (with zero bias), not self-test */
   if(conf(CFG_BIAS_NONE)) return HMC5883L_ERR_I2C;

   /* If the gain was changed, the next reading will use the old gain.
      Perform a reading and discard the results so the caller doesn't
      have to worry about this. */
   return hmc5883l_read(&buf);
}

/*** hmc5883l_test() -- perform self-test and get scaled result

Performs a (positive-bias) self-test measurement, and returns the result.

The HMC5883L sensor has a built-in self-test mode. In this mode, it takes
two measurements: one as usual, and one with current flowing through an
offset strap to superimpose a ~1.1Ga field on the existing environmental
field. The result is the difference of the two values (which should fall
within known limits, regardless of the strength or direction of the
environmental field).

The self-test measurement is performed for each of the three axes.

Arguments:
   p -- pointer to buffer into which result data are to be placed

Returns one of the HMC5883L_ERR_* values.

Note that the results of a (successful) self-test can be used as
scaling factors for temperature compensation. See data sheet for details.
***/
int hmc5883l_test(hmc5883l_pos_t * p) {
   int rtn;
   int16_t min, max;

   /* Go into self-test mode, take a reading, then leave self-test mode
      (even if the reading failed). */
   if(conf(CFG_BIAS_POS)) return HMC5883L_ERR_I2C;
   rtn = hmc5883l_read(p);
   if(conf(CFG_BIAS_NONE)) return HMC5883L_ERR_I2C;

   /* If the reading failed, return that same failure to the caller: */
   if(rtn != HMC5883L_ERR_OK) return rtn;

#if CFG_GAIN == CFG_GAIN_1370
#define GAIN_MULT 1370
#elif CFG_GAIN == CFG_GAIN_1090
#define GAIN_MULT 1090
#elif CFG_GAIN == CFG_GAIN_820
#define GAIN_MULT 820
#elif CFG_GAIN == CFG_GAIN_660
#define GAIN_MULT 660
#elif CFG_GAIN == CFG_GAIN_440
#define GAIN_MULT 440
#elif CFG_GAIN == CFG_GAIN_390
#define GAIN_MULT 390
#elif CFG_GAIN == CFG_GAIN_330
#define GAIN_MULT 330
#elif CFG_GAIN == CFG_GAIN_230
#define GAIN_MULT 230
#else
#error self-test limits are missing for selected CFG_GAIN value
#endif /* if/else CFG_GAIN values */
#define GAIN_DIV 390 /* LSb per Gauss for SELFTEST_POS_MIN and MAX */

   min = SELFTEST_POS_MIN * GAIN_MULT / GAIN_DIV;
   max = SELFTEST_POS_MAX * GAIN_MULT / GAIN_DIV;

   if(p->x < min || p->x > max ||
      p->y < min || p->y > max ||
      p->z < min || p->z > max)     return HMC5883L_ERR_TESTFAIL;
   
   return HMC5883L_ERR_OK;
}
