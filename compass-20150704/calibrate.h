/* Copyright (c) 2014-2015 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#ifndef CALIBRATE_H
#define CALIBRATE_H

#include <hmc5883l/hmc5883l.h>
#include "rotate.h"

typedef struct {
   hmc5883l_pos_t scale; /* self-test for temperature compensation */
   hmc5883l_pos_t xlate;
   rotation_t rotate;
} calibration_t;

int calibrate(calibration_t * const p) __attribute__((nonnull(1)));

#endif /* ifndef CALIBRATE_H */
