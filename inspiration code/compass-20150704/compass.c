/* Copyright (c) 2014-2015 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <uWireM/uWireM.h>
#include <matrix8x8/matrix8x8.h>
#include <hmc5883l/hmc5883l.h>
#include "button.h"
#include "calibrate.h"
#include "heading.h"
#include "stored_cal.h"
#include "tempcomp.h"
#include "validate.h"

#define BRIGHTNESS 10 /* 0=dim, 15=max */
#define ARC (FIXEDPT_BRAD_SEMICIRC >> 4) /* one 32nd of a circle */
#define TCOMP_PERIOD 6000

/*** flash_err() -- blink "ERR" indicator at 1Hz; stop on button press

Flashes the "ERR" indication on and off at 1Hz until the user presses
the button.
***/
static void flash_err(void) {
   uint8_t img[8];

   memset(img, 0, 8);
   img[2] = 0b10000000; img[3] = 0b10000000; img[4] = 0b10000000;
   while(1) {
      matrix8x8_draw(img);
      if(button_sleep(500)) return;
      matrix8x8_clear();
      if(button_sleep(500)) return;
   }
}

int main(void) {
   int rtn;
   calibration_t calib;
   hmc5883l_pos_t p, tcal;
   fixedpt_t hdg, prev = 0;
   uint16_t tcomp_cnt = 0;
   uint8_t img[8];

   power_timer1_disable();  /* turn off to save power */
   power_adc_disable();     /* likewise */
   BTN_DDR &= ~BTN_DDRBIT;  /* make button pin an input */
   BTN_PORT |= BTN_PORTBIT; /* activate internal pull-up on button pin */

   uWireM_init();              /* set up I2C communication library */
   matrix8x8_init(BRIGHTNESS); /* set up 8x8 LED matrix */
   hmc5883l_init();            /* set up magnetometer */

   /* If there is no calibration data in the EEPROM, force calibration
      before we proceed: */
   if(stored_cal_get(&calib) < 0) {
      while(calibrate(&calib)) flash_err();
      stored_cal_set(&calib);
      memcpy(&tcal, &(calib.scale), sizeof(hmc5883l_pos_t));
      tcomp_cnt = TCOMP_PERIOD;
   }

   while(1) {
      if(button()) { /* if button is pressed, attempt re-calibration */
         /* attempt calibration; store in EEPROM on success */
         if((rtn = calibrate(&calib)) == 0) {
            stored_cal_set(&calib);
            memcpy(&tcal, &(calib.scale), sizeof(hmc5883l_pos_t));
            tcomp_cnt = TCOMP_PERIOD;
         } else { /* calibration failed */
            stored_cal_get(&calib); /* get previous settings */
            /* If this is an actual error and not just the user cancelling
               the calibration, flash the ERR indicator on the display
               and wait for a button press. */
            if(rtn < 0) flash_err();
         }
      }

      memset(img, 0, 8);

      /* get a raw compass reading; detect and display any errors */
      if((rtn = hmc5883l_read(&p)) == HMC5883L_ERR_SATURATED) {
         img[3]=0b00000010; img[4]=0b00000010; /* "INTF" */
         img[5]=0b00000010; img[6]=0b00000010;
         matrix8x8_draw(img);
         continue; 
      } else if(rtn != HMC5883L_ERR_OK) {
         img[2] = 0b10000000; img[3] = 0b10000000; img[4] = 0b10000000;
         matrix8x8_draw(img); /* "ERR" */
         continue; 
      }

      /* If we haven't gotten temperature compensation data in a while,
         get some and reset the counter. Otherwise, decrement the counter. */
      if(tcomp_cnt == 0) {
         if(hmc5883l_test(&tcal) != HMC5883L_ERR_OK) {
            img[2] = 0b10000000; img[3] = 0b10000000; img[4] = 0b10000000;
            matrix8x8_draw(img); /* "ERR" */
            continue;
         }
         tcomp_cnt = TCOMP_PERIOD;
      } else tcomp_cnt--;

      /* temperature-compensate the raw reading */
      tempcomp(&p, &tcal, &(calib.scale));

      /* Offset and rotate reading per calibration data. Calculate
         heading. */
      hdg = heading(&p, &calib);

      /* Validate adjusted reading and display error indication if
         warranted: */
      if((rtn = validate(&p)) != VLD_ERR_OK) {
         if(rtn & VLD_ERR_TILT) {
            img[2] |= 0b00000001; img[3] |= 0b00000001;
            img[4] |= 0b00000001; img[5] |= 0b00000001;
         }
         if(rtn & VLD_ERR_INTF) {
            img[3] |= 0b00000010; img[4] |= 0b00000010; /* "INTF" */
            img[5] |= 0b00000010; img[6] |= 0b00000010;
         }
         matrix8x8_draw(img);
         continue;
      }

#ifdef FORGET_THIS_FOR_NOW
      // FIXME: put something sensible on top line
      img[0] = ((hdg >> 4) & 0x00ff);
#endif /* ifdef FORGET_THIS_FOR_NOW */

      /* Add a slight hysteresis to the heading to prevent display flickering
         when we're just on the border between two adjacent readings: */
      if(fxp_abs(prev-hdg) < (ARC >> 3)) hdg = prev;
      else prev = hdg;

      if(hdg <= ARC*7 || hdg > ARC*25) { /* Northerly 7 directions */
         img[5]=0b01111100; /* NORTH */
         /* If NNE or NNW, add the extra "NORTH" prefix... */
         if((hdg > ARC*29 && hdg <= ARC*31) ||
          (hdg > ARC && hdg <= ARC*3)) img[3] = 0b01111100;

      } else if(hdg > ARC*9 && hdg <= ARC*23) { /* Southerly 7 */
         img[6]=0b01111100; /* SOUTH */
         if((hdg > ARC*13 && hdg <= ARC*15) ||
          (hdg > ARC*17 && hdg <= ARC*19)) img[4] = 0b01111100; /* S prefix */
      }

      if(hdg > ARC && hdg <= ARC*15) { /* Easterly 7 */
         img[7]=0b11110000; /* EAST */
         if((hdg > ARC*5 && hdg <= ARC*7) ||
          (hdg > ARC*9 && hdg <= ARC*11)) img[2] = 0b11110000; /* E prefix */
      } else if(hdg > ARC*17 && hdg <= ARC*31) { /* Westerly 7 */
         img[7]=0b00001111; /* WEST */
         if((hdg > ARC*21 && hdg <= ARC*23) ||
          (hdg > ARC*25 && hdg <= ARC*27)) img[2] = 0b00001111; /* W prefix */
      }

      matrix8x8_draw(img);
   }
}
