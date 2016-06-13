/* Copyright (c) 2014 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <linux/i2c-dev.h>
#include <arpa/inet.h>
#include "diag.h"
#include "debug.h"
#include "i2c_ops.h"
#include "hmc5883l.h"

/* Register addresses on HMC5883L: */
#define REG_CRA 0  /* config A */
#define REG_CRB 1  /* config B */
#define REG_MR 2   /* mode */
#define REG_DATA 3 /* registers 3-8 inclusive are data, read as a block */
#define REG_SR 9   /* status */
#define REG_IRA 10 /* ID A = 'H' */
#define REG_IRB 11 /* ID B = '4' */
#define REG_IRC 12 /* ID C = '3' */

/*** hmc5883l_chk() -- check for HMC5883L magnetometer on I2C bus

This callback function is intended for use with i2c_find() to test for
the presence of a Honeywell HMC5883L 3-axis digital compass on the I2C
bus.

The argument is a file descriptor open on an I2C bus an initialized for
communication with a slave device at 0x1E.

The author does not know of any I2C devices using 7-bit address 0x1E which
react badly to this probe. (Nor, indeed, does he know of any other I2C
devices which use address 0x1E.)

Returns 0 IFF an HMC5883L device is found on the bus associated with
the given descriptor.
***/
static int hmc5883l_chk(const int fd) {
   TSTA(fd < 0);
   if(i2c_smbus_read_byte_data(fd, REG_IRA) != 'H') return -1;
   if(i2c_smbus_read_byte_data(fd, REG_IRB) != '4') return -1;
   if(i2c_smbus_read_byte_data(fd, REG_IRC) != '3') return -1;
   return 0;
}

/*** hmc5883l_open() -- open fd to talk to HMC5883L

Locates an HMC5883L magentometer on one of the available I2C busses and
returns an open descriptor prepared to communicate with it. (If there are
several busses with an HMC5883L connected, one will be chosen arbitrarily.
But why would that situation exist?)

The caller is responsible for close()(2)-ing the descriptor when it is no
longer needed.

On error (or if no such device could be located) returns a negative value.
***/
int hmc5883l_open(void) {
   return i2c_find(0x1e, "HMC5883L", hmc5883l_chk);
}

/*** hmc5883l_read() -- read sensor values from HMC5883L

Read the sensor values from the HMC5883L.

Note: Results are undefined if HMC5883L has not yet been configured
using hmc5883l_config().

Arguments:
   fd -- descriptor returned by previous call to hmc5883l_open()
   p -- pointer to data structure into which sensor data is to be
      stored, or NULL if the readings are to be discarded
   gain -- gain value used to convert raw readings to Gauss; must
      be the same as that in the gain field of the hmc5883l_config_t
      structure passed to hmc5883l_config()

Returns 0 on success, non-0 on failure.
***/
int hmc5883l_read(hmc5883l_state_t * const p, hmc5883l_pos_t * const pos) {
   int i;
   static int dbg = 0;
   __u8 buf[6];
   int16_t *val = (int16_t *)buf;
#ifdef NEVER
   static const int bits_per_gauss[] = { /* indices are enum gain values */
      1370, 1090, 820, 660, 440, 390, 330, 230
   };
#endif

   TSTA(!p); TSTA(p->fd < 0);

   /* Start a measurement in single measurement mode: */
   if(i2c_smbus_write_byte_data(p->fd, REG_MR, 0x01))
      return sysdiag("i2c_smbus_write_byte_data", "can't write to Mode");

   /* Tell the device we want to start reading at DXRA (register 3): */
   if(i2c_smbus_write_byte(p->fd, REG_DATA))
      return sysdiag("i2c_smbus_write_byte", "failed writing position");

   /* We could poll the status register here, but on a time-sharing
      system, we could easily miss the very brief period during which
      it's low. So instead, we'll just block for a minimum of
      (1Hz/s)/160Hz = 6250us, after which we know the reading will be
      ready. */
   usleep(6250);

   /* Read all six positions: */
   if(read(p->fd, buf, 6) != 6)
      return sysdiag("read", "failed reading position values");

   if(!pos) return 0;

   // FIXME: not needed on AVR (?)
   for(i = 0; i < 3; i++) val[i] = ntohs(val[i]);

   pos->x = val[0];
   pos->y = val[1];
   pos->z = val[2];
   if(dbg == 0) {
//diag("DBG: x=%.3f y=%.3f z=%.3f", flt(pos->x), flt(pos->y), flt(pos->z));
      dbg = 100;
   } else dbg--;

   return 0;
}

/*** hmc5883l_config() -- configure HMC5883L sensor

Performs initialization and configuration on HMC5883L sensor.

Arguments:
   fd -- descriptor returned by previous call to hmc5883l_open()
   cfg -- pointer to structure containing desired configuration, or
      NULL to use the default configuration from _hmc5883l_config_default

Returns 0 on success or non-0 on error.
***/
int hmc5883l_config(hmc5883l_state_t * const p) {
   TSTA(!p); TSTA(p->fd < 0);

   /* Set config register A for desired averaging and bias. We leave
      the output rate all-bits-zero since we're just going to be using
      single measurement mode: */
   if(i2c_smbus_write_byte_data(p->fd, REG_CRA,
    (p->cfg.avg)<<5 | (p->cfg.bias)))
      return sysdiag("i2c_smbus_write_byte_data", "can't write to CRA");

   /* Set config register B for desired gain: */
   if(i2c_smbus_write_byte_data(p->fd, REG_CRB, (p->cfg.gain)<<5))
      return sysdiag("i2c_smbus_write_byte_data", "can't write to CRB");

   /* Set mode register to force idle mode: */
   if(i2c_smbus_write_byte_data(p->fd, REG_MR, 0x03))
      return sysdiag("i2c_smbus_write_byte_data", "can't write to Mode");

   /* If the gain was changed, the next reading will use the old gain.
      Perform a reading and discard the results so the caller doesn't
      have to worry about this. */
   return hmc5883l_read(p, NULL);
}
