/* Copyright (c) 2014-2015 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */

/* IMPORTANT: This code assumes that it is the *only* thing using the EEPROM
   for anything. */

/* NOTE: Linking against this module will inflate the size of the "data"
   section reported by avr-size by approximately the size of your EEPROM.
   Don't worry about this; it doesn't actually count against you in
   terms of flash memory or RAM. */

/* NOTE: You don't need to set up EEPROM any special way when flashing.
   When in doubt -- and always, on factory-new parts -- leave the EESAVE
   fuse bit cleared. The code will do the Right Thing when it sees a
   completely empty (all bits one) EEPROM. */

/* This module deals with storing calibration data in the EEPROM so that
   the device will retain calibration across loss of power.

   EEPROM cells have a limited lifetime in terms of the number of times
   you can write to them. Per the manufacturer, this is about 100K cycles
   per cell.

   So, to make the EEPROM last as long as possible, we divide up the
   available space into slots, each big enough to hold the calibration
   data. We use the slots in rotation, starting the slot 0 and working
   up, then wrapping around back to zero when we get to the end.

   Any slot not holding the current calibration data will be in the
   default, just-erased state: all bits 1. (This is not a valid calibration
   state, so it's easy to tell apart from a used slot.)

   On an ATtiny85 with 512 bytes of EEPROM, we can fit 17 slots of
   calibration data (at 30 bytes each). Since each update touches two
   slots (clearing the old one and updating the new one), and the slots
   are used in rotation, this should make the EEPROM last 8.5 times
   as long -- 850K cycles instead of 100K.
*/

#include <string.h>
#include <avr/eeprom.h>
#include "calibrate.h"
#include "stored_cal.h"

/* Decide the EEPROM capacity in bytes based on the target platform: */
#ifdef __AVR_ATtiny85__
#define EEPROM_SIZE 512
#else
#error EEPROM_SIZE not defined for this target
#endif

/* Divide the available EEPROM space into slots, each one of which is
   big enough to hold a copy of the calibration data: */
#define SLOTS (EEPROM_SIZE / sizeof(calibration_t))

static calibration_t EEMEM _cal[SLOTS];

/*** stored_cal_get() -- get calibration data from EEPROM, if present

Retrieves the current calibration data from EEPROM, if any calibration
data are present there.

The argument is a pointer to a buffer into which the calibration data
should be copied, or NULL if no copy is required.

Returns the zero-based slot index in which the current calibration data
are stored, or a negative value if EEPROM contains no calibration data.
***/
int stored_cal_get(calibration_t * const dst) {
   int i, j;
   calibration_t buf;
   uint8_t *p = (uint8_t *)&buf;

   /* Read each slot in turn. (Reads do not wear out the EEPROM.) */
   for(i = 0; i < SLOTS; i++) {
      eeprom_read_block(&buf, _cal+i, sizeof(buf));

      /* If any byte is something other than all-bits-1, then we
         have found the right slot; break out of the outer loop. */
      for(j = 0; j < sizeof(buf); j++) if(p[j] != 0xff) break;
      if(j < sizeof(buf)) break;
   }

   if(i == SLOTS) return -1; /* EEPROM is entirely empty */

   if(dst) memcpy(dst, &buf, sizeof(buf));
   return i;
}

/*** stored_cal_set() -- write calibration data to EEPROM, with wear leveling

Writes a set of calibration data to EEPROM, in such a way as to allow those
data to be retrieved with stored_cal_get() (even after a power failure or
reset event).

The argument is a pointer to a buffer containing the calibration data to
be stored.
***/
void stored_cal_set(calibration_t * const src) {
   int i, j;
   calibration_t buf;
   uint8_t *p = (uint8_t *)&buf;

   /* Find out what slot contains the current calibration data, or if
      EEPROM is totally empty. */
   if((i = stored_cal_get(NULL)) >= 0) {

      /* Clear the slot containing the current calibration data.
         (We need to do this by creating a buffer that's all-bits-1
         then updating it into the slot. This would be easier if the
         library exposed the native "clear" instruction.) */
      for(j = 0; j < sizeof(buf); j++) p[j] = 0xff;
      eeprom_update_block(&buf, _cal+i, sizeof(buf));

      /* Figure out what slot to use to store the new calibration data.
         Normally, this is the slot after the one we just cleared. If
         there is no slot after the one we just cleared, start over at
         slot zero. */
      if(++i == SLOTS) i = 0; 

   } else i = 0; /* If EEPROM is empty, just start with slot zero. */
 
   /* Write the new calibration data into the next slot: */ 
   eeprom_update_block(src, _cal+i, sizeof(buf));
}
