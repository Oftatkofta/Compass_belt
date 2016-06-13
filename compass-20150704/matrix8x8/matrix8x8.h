/* Copyright (c) 2014-2015 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#ifndef MATRIX8X8_H
#define MATRIX8X8_H

/* See README for introduction and licensing information. */

/* See matrix8x8.c for API documentation. */

/* For more details on the commmands, see the HTK16K33 data sheet:
   http://www.adafruit.com/datasheets/ht16K33v110.pdf */

#define MATRIX8X8_I2C_ADDR 0x70 /* default; change if A0/A1 jumpers shorted */

#include <stdint.h>

/* Command bytes. These are all the documented commands for the controller
   chip, some of which are useless in the context of the Adafruit LED
   backpack. The only ones we'll use are DISPLAY_XFER, SYSTEM_SETUP,
   DISPLAY_SETUP and DISPLAY_DIM. */
#define MATRIX8X8_CMD_DISPLAY_XFER  0x00 /* RW; low 4 bits = start addr */
#define MATRIX8X8_CMD_SYSTEM_SETUP  0x20 /* W; low bit = oscillator on/off */
#define MATRIX8X8_CMD_KEYBD_READ    0x40 /* R; low 3 bits = start addr */
#define MATRIX8X8_CMD_INT_FLAG_READ 0x60 /* R; get INT flag state */
#define MATRIX8X8_CMD_DISPLAY_SETUP 0x80 /* W; bit 0=on/off; bits 1-3=blink */
#define MATRIX8X8_CMD_ROW_INT_SETUP 0xA0 /* W; bits 0,1 = state */
#define MATRIX8X8_CMD_DISPLAY_DIM   0xE0 /* W; low 4 bits = brightness */
#define MATRIX8X8_CMD_TEST_MODE     0xD9 /* W; "HOLTEK use only" */

#define MATRIX8X8_PWR_ON 1
#define MATRIX8X8_PWR_STANDBY 0
#define matrix8x8_pwr(x) matrix8x8_cmd(MATRIX8X8_CMD_SYSTEM_SETUP|x)

/* The dimming uses PWM, and is adjustable from 1:16 duty cycle up to
   100% (in steps of 1:16). You can't turn the display completely off
   with the dim command; clear it, set the mode to OFF or set the power
   to standby if you want that. */
#define MATRIX8X8_DIM_1_16 0
#define MATRIX8X8_DIM_2_16 1
#define MATRIX8X8_DIM_3_16 2
#define MATRIX8X8_DIM_4_16 3
#define MATRIX8X8_DIM_5_16 4
#define MATRIX8X8_DIM_6_16 5
#define MATRIX8X8_DIM_7_16 6
#define MATRIX8X8_DIM_8_16 7
#define MATRIX8X8_DIM_9_16 8
#define MATRIX8X8_DIM_10_16 9
#define MATRIX8X8_DIM_11_16 10
#define MATRIX8X8_DIM_12_16 11
#define MATRIX8X8_DIM_13_16 12
#define MATRIX8X8_DIM_14_16 13
#define MATRIX8X8_DIM_15_16 14
#define MATRIX8X8_DIM_16_16 15
#define MATRIX8X8_DIM_FULL MATRIX8X8_DIM_16_16
#define MATRIX8X8_DIM_HALF MATRIX8X8_DIM_8_16
#define matrix8x8_dim(x) matrix8x8_cmd(MATRIX8X8_CMD_DISPLAY_DIM|x)

#define MATRIX8X8_MODE_ON 1
#define MATRIX8X8_MODE_OFF 0
#define MATRIX8X8_MODE_BLINK_2HZ 3
#define MATRIX8X8_MODE_BLINK_1HZ 5
#define MATRIX8X8_MODE_BLINK_HALF_HZ 7
#define matrix8x8_mode(x) matrix8x8_cmd(MATRIX8X8_CMD_DISPLAY_SETUP|x)

/* matrix8x8_init() is a macro to simplify initialization. It clears
   the display memory, turns on the oscillator, sets the brightness,
   and sets the mode to "on" (with no blink). The single argument is
   the desired dimming level -- probably one of the MATRIX8X8_DIM_*
   defines above. */
#define matrix8x8_init(x) \
   matrix8x8_clear();\
   matrix8x8_pwr(MATRIX8X8_PWR_ON);\
   matrix8x8_dim(x);\
   matrix8x8_mode(MATRIX8X8_MODE_ON);

__attribute__((pure)) uint8_t matrix8x8_pixels(uint8_t x);
void matrix8x8_cmd(const uint8_t x);
void matrix8x8_clear(void);
void matrix8x8_drawP(const uint8_t * const p);
void matrix8x8_draw(const uint8_t * const p);

#endif /* ifndef MATRIX8X8_H */
