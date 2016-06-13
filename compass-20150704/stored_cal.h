/* Copyright (c) 2014-2015 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#ifndef STORED_CAL_H
#define STORED_CAL_H

#include "calibrate.h"

int stored_cal_get(calibration_t * const dst);
void stored_cal_set(calibration_t * const src);

#endif /* ifndef STORED_CAL_H */
