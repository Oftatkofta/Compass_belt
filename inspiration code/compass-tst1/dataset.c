/* This is free software. See COPYING for details. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "dynlist.h"
#include "fgetrec.h"
#include "diag.h"
#include "debug.h"

/* A pos_t is just a point in 3-space represented using rectangular
   coordinates: */
typedef struct { double x,y,z; } pos_t;

/*** hdl_rec() -- handle input record (by parsing and adding to list)

This function is used as a callback with fgetrec.c:for_each_rec(). It
is called once for each line in the input. It expects the line to contain
three floating-point numbers (X, Y and Z) separated by spaces.

If the line is in the correct format, a pos_t representing the corresponding
position is added to the dynlist_t pointed to by the userdata.

Returns 0 on success, or non-0 on error. (Badly-formed input lines count
as an error.)
***/
static int hdl_rec(const void * const rec, const int len, const int line,
 const char * const fname, void * const userdata) {
   pos_t pos;
   dynlist_t *lst = (dynlist_t *)userdata;

   if(sscanf(rec, "%lf %lf %lf\n", &(pos.x), &(pos.y), &(pos.z)) != 3)
      return diag("%s:%d malformatted line", fname, line);
   return dynlist_add(lst, &pos);
}

/*** dataset_read() -- read data set from input file

Reads a set of three-dimensional positions from an input file. See
hdl_rec() above for details of the syntax.

On success, returns a pointer to a dynlist_t of pos_t items read from the
input file (in the order they were read). The caller must dynlist_free()
the returned pointer when the list contents are no longer needed.

On failure, returns NULL.
***/
dynlist_t *dataset_read(const char * const fname) {
   dynlist_t *lst;
   int rtn = 0;
   rec_handler_t handlers[2];

   TSTA(!fname);

   memset(handlers, 0, sizeof(handlers));

   handlers[0].len_min = 6; handlers[0].len_max = 127;
   handlers[0].handler = hdl_rec;

   if((lst = dynlist_new(sizeof(pos_t), 1024)) == NULL) { rtn = -1; goto done; }
   rtn = for_each_rec(fname, 128, handlers, lst);

   done:
   if(rtn && lst) { dynlist_free(lst); lst = NULL; }
   return lst;
}

/*** dataset_foreach() -- iterate over points, calling callback once for each

Iterates over the list of pos_t items in the dynlist_t pointed to by the
first argument, in order.

For each point, calls the callback function pointed to by the second argument.

The third argument is a pointer to arbitrary data, which is passed through
to the callback function. (This may be set to NULL if not needed.).

The callback function takes the following arguments:
   i -- 0-based index of point being processed
   pos -- pointer to position of current point
   userdata -- the userdata argument passed to dataset_foreach()

The callback function is expected to return 0 on success, non-0 on error.

If the callback function returns a non-0 value, processing stops immediately
and the same value is returned by dataset_foreach().

Otherwise, dataset_foreach() returns 0 when all points have been processed.
***/
int dataset_foreach(dynlist_t * const lst, int(*hdl)(const int i,
 pos_t * const pos, void * const userdata), void * const userdata) {
   int i, rtn;
   pos_t *p;

   TSTA(!lst); TSTA(!hdl);

   for(i = 0; i < lst->len; i++) {
      p = (pos_t *)(lst->lst) + i;
      if((rtn = hdl(i, p, userdata)) != 0) return rtn;
   }
   return 0;
}

/*** dumpone() -- dump a single position to stdout

Writes a single position to stdout, in the format understood by
dataset_read(). This function is intended for use as a callback
by dataset_foreach() (which see).

The first and third arguments (i and userdata) are ignored. Always
returns 0.
***/
static int dumpone(const int i, pos_t * const pos, void * const userdata) {
   printf("%lf %lf %lf\n", pos->x, pos->y, pos->z);
   return 0;
}

/*** dataset_dump() -- dumps data set to stdout

Dumps a set of points to stdout, in the format understood by dataset_read().
The argument is a pointer to a dynlist_t of pos_t elements.

Returns 0 on success, non-0 on error.
***/
int dataset_dump(const dynlist_t * const lst) {
   TSTA(!lst);
   return dataset_foreach((dynlist_t *)lst, dumpone, NULL);
}
