/* Copyright (c) 2014-2015 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#ifndef HMC5883L_H
#define HMC5883L_H

#include "../fixedpt.h"

/* possible return values from read, test and init functions: */
#define HMC5883L_ERR_OK 0            /* no error */
#define HMC5883L_ERR_I2C (-1)        /* unable to communicate with sensor */
#define HMC5883L_ERR_SATURATED (-2)  /* field too strong; sensor saturated */
#define HMC5883L_ERR_TESTFAIL (-3)   /* self-test failed */

typedef struct { fixedpt_t x, y, z; } hmc5883l_pos_t;

int hmc5883l_read(hmc5883l_pos_t * const p) __attribute__((nonnull(1)));
int hmc5883l_init(void);
int hmc5883l_test(hmc5883l_pos_t * const p) __attribute__((nonnull(1)));

#endif /* ifndef HMC5883L_H */
