/* Copyright (c) 2014 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "diag.h"
#include "debug.h"
#include "hmc5883l.h"
#include "calibrate.h"
#include "heading.h"

int main(void) {
   hmc5883l_state_t state;
   hmc5883l_pos_t pos;
   calibration_t calib;

   diag("Initializing... please hold still.");

   /* Set up the sensor: */
   state.cfg.avg = AVG_8;
   //state.cfg.gain = GAIN_1370;
   state.cfg.bias = BIAS_NONE;
   state.cfg.gain = GAIN_1090;
   if((state.fd = hmc5883l_open()) < 0) return -1;
   if(hmc5883l_config(&state)) return -1;

   /* Get calibration data: */
   calibrate(&state, &calib);

   diag("Ready to read...");

   while(1) {
      if(hmc5883l_read(&state, &pos)) return -1;
      //diag("HDG: %.3f", flt(heading(&pos, &calib)) * 180.0 / 8); /* works */
      diag("HDG: %.3f", flt(heading(&pos, &calib)) * 180.0 /
       (FIXEDPT_BRAD_SEMICIRC >> FIXEDPT_FRACBITS) );
      //diag("HDG: %f", 180.0*heading(&(state.pos), &calib)/M_PI);
   }

   if(close(state.fd)) return sysdiag("close", "can't close I2C bus");
   return 0;
}
