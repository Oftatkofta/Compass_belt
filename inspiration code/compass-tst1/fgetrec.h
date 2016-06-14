#ifndef FGETREC_H
#define FGETREC_H

#include <stdio.h> /* for FILE definition */

char *fgets_nonul(char * const s, const int size, FILE * const stream);
int fgetrec(const char * const name, FILE * const in, char * const buf,
 const size_t len, int * const line);

typedef struct {
   int len_min, len_max, sig_offset, sig_len;
   char *sig;
   int(*handler)(const void * const rec, const int len, const int line,
    const char * const fname, void * const userdata);
   int(*handler2)(const void * const rec, const int len, const int line,
    const char * const fname, void * const userdata, FILE * const in,
    const long offset);
} rec_handler_t;

int for_each_rec(const char * const name, const int recsize,
 const rec_handler_t * const handler_lst, void * const userdata);

int for_each_rec_fixed(const char * const name, const int recsize,
  int(*handler)(const void * const rec, const int len, const int line,
  const char * const fname, void * const userdata), void * const userdata);

#endif /* ifndef FGETREC_H */
