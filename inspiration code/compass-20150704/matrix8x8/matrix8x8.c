/* Copyright (c) 2014-2015 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#include <avr/pgmspace.h>
#include <string.h>
#include <uWireM/uWireM.h>
#include "matrix8x8.h"

static uint8_t _buf[18];

/*** matrix8x8_pixels() -- turn a bitmap byte into a display controller row

Converts a bitmap byte (in bit order 76543210) into a byte in the bit
order the display controller expects (70123456).

The argument is the bitmap byte. Returns the converted value.

This function could be eliminated (saving a dozen or so bytes), but
it would mean sacrificing a more human-readable presentation of the
bitmap images (i.e., 0b10000000 means the leftmost pixel in the row
is lit) or using some kind of compile-time preprocessor.

The same operation could be done faster with a lookup table, but we
really care more (a *lot* more!) about space than speed here.
***/
__attribute__((pure)) uint8_t matrix8x8_pixels(uint8_t x) {
   /* Reverse the order of the bits in byte x: */
   x = x >> 4 | x << 4;
   x = (x & 0xCC) >> 2 | (x & 0x33) << 2;
   x = (x & 0xAA) >> 1 | (x & 0x55) << 1;

   /* Rotate the whole thing right one place, putting what used to
      be the least significant bit back in at the top. */
   return (x >> 1) | (x << 7);
}

/*** matrix8x8_cmd() -- send command to LED driver

Sends a single-byte command to the LED driver. Examples of such commands
would be those to set the brightness or blink mode.

The argument is the command value to be sent.
***/
void matrix8x8_cmd(const uint8_t x) {
   _buf[0] = uWireM_addr_send(MATRIX8X8_I2C_ADDR);
   _buf[1] = x;
   uWireM_xfer(_buf, 2);
}

/*** matrix8x8_clear() -- turn all LEDs off by writing 8 empty rows

Blanks the LED display by populating all the rows with all-pixels-off data.
***/
void matrix8x8_clear(void) {
   /* Why bother clearing _buf[0] if we're going to immediately set it
      to another value? Avoiding doing so would make the code bigger (and
      probably slower, too). */
   memset(_buf, 0, sizeof(_buf));
   _buf[0] = uWireM_addr_send(MATRIX8X8_I2C_ADDR);
   uWireM_xfer(_buf, 18);
}

/*** matrix8x8_drawP() -- draw single frame from program memory

Draws a single frame from program memory on the LED matrix display.

The argument is a pointer to the byte representing the first (top) row of
the bitmap to be drawn. Subsequent rows are assumed to be contiguous in
memory.
***/
void matrix8x8_drawP(const uint8_t * const p) {
   uint8_t row;

   /* Again, we zero the whole transfer buffer then selectively replace
      parts of it in the interests of saving code space: */
   memset(_buf, 0, sizeof(_buf));
   _buf[0] = uWireM_addr_send(MATRIX8X8_I2C_ADDR);

   /* The rows are numbered 0..8 from top to bottom. The row data goes
      into the _buf[] elements with even-numbered indices, starting with
      _buf[2] for the top row, going up to _buf[16] for the bottom. */
   for(row = 0; row < 8; row++)
      _buf[(row+1)<<1]= matrix8x8_pixels(pgm_read_byte(p+row));

   uWireM_xfer(_buf, 18);
}

/*** matrix8x8_drawP() -- draw single frame from SRAM

This function is almost idential to matrix8x8_drawP(), above. However,
it takes the image data from SRAM instead of program memory.
***/
void matrix8x8_draw(const uint8_t * const p) {
   uint8_t row;
   memset(_buf, 0, sizeof(_buf));
   _buf[0] = uWireM_addr_send(MATRIX8X8_I2C_ADDR);
   for(row = 0; row < 8; row++)
      _buf[(row+1)<<1]= matrix8x8_pixels(*(p+row));
   uWireM_xfer(_buf, 18);
}
