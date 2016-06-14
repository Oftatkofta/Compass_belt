/* Copyright (c) 2014 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#ifndef DIAG_H
#define DIAG_H

#include <stdarg.h>

int diag(char *fmt, ...) __attribute__ ((format(printf,1,2)));
void *diagp(char *fmt, ...) __attribute__ ((format(printf,1,2)));
int sysdiag(char *syscallname, char *fmt, ...)
 __attribute__ ((format(printf,2,3)));
void *sysdiagp(char *syscallname, char *fmt, ...)
 __attribute__ ((format(printf,2,3)));

#endif /* ifndef DIAG_H */
