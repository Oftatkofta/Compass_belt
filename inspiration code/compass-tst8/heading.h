/* Copyright (c) 2014 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#ifndef HEADING_H
#define HEADING_H

#include "hmc5883l.h"
#include "calibrate.h"
#include "fixedpt.h"

fixedpt_t heading(const hmc5883l_pos_t * const pos,
 const calibration_t * const calib);

#endif /* ifndef HEADING_H */
