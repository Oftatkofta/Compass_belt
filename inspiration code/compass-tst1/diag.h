#ifndef DIAG_H
#define DIAG_H

#include <stdarg.h>

int diag(char *fmt, ...);
void *diagp(char *fmt, ...);
int sysdiag(char *syscall, char *fmt, ...);
void *sysdiagp(char *syscall, char *fmt, ...);

#endif /* ifndef DIAG_H */
