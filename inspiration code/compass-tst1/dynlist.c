/* This is free software. See COPYING for details. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "diag.h"
#include "debug.h"
#include "dynlist.h"

typedef struct {
   int max, len, size, delta;
   void *lst;
} list_t;

/***
dynlist_new() -- create a new dynamic list of generic uniform-size items

Creates a new, dynamically-allocated data structure capable of holding an
ordered list of items, where all items have the same (arbitrary) size. The
size in bytes of each list item is given by the first argument. The new list
is initially empty.

The second argument is the number of elements by which the list grows
when more space is required. The optimal value depends on the size of
each element and the distribution curve of the peak number of elements
needed.

Returns a pointer to the new list on successs, NULL on error. The caller
is responsible for cleanup (by calling list_free(), below) when the list is
no longer needed.
***/
dynlist_t *dynlist_new(const int size, const int delta) {
   dynlist_t *new;

   TSTA(size <= 0); TSTA(delta <= 0);

   if((new = malloc(sizeof(*new))) == NULL)
      return sysdiagp("malloc", "can't allocate new list");
   memset(new, 0, sizeof(*new));
   new->size = size;
   new->delta = delta;
   return new;
}

/***
dynlist_free() -- deallocates a list previously created with dynlist_new()

Deallocates all dynamic storage associated with the dynlist_t pointed to
by the argument. Returns 0 on success, non-0 on error.
***/
int dynlist_free(dynlist_t * const p) {
   if(!p) return 0;
   if(p->lst) free(p->lst);
   free(p);
   return 0;
}

/***
dynlist_add() -- add a new item to the end of a dynlist_t

Adds a new item to the end of the dynlist_t pointed to by the first argument.
The second argument points to the first byte of the item to be added.

Returns 0 on success, non-0 on error.
***/
int dynlist_add(dynlist_t * const p, const void * const item) {
   int max;
   void *new;

   TSTA(!p); TSTA(!item);
   TSTA(p->len < 0); TSTA(p->max < 0);
   TSTA(p->len > p->max);

   if(p->len == p->max) {
      max = p->max + p->delta;
      if((new = realloc(p->lst, p->size * max)) == NULL)
         return sysdiag("realloc", "can't grow list");
      p->max = max; p->lst = new;
   }

   memcpy(p->lst + p->size * p->len, item, p->size);
   (p->len)++;
   return 0;
}

/*** dynlist_qsort() -- qsort()(3) elements of a dynamic list
Sorts the elements of a dynlist_t using qsort()(3). The first argument is
a pointer to the dynlist_t to be sorted. The second argument is a pointer
to a comparison function a la qsort()(3).

Note that unlink some qsort() implementations this function is guaranteed
to be harmless if the list is empty (p->len == 0). Also note that the
comparison function must be stable. ("Stable" here means that if a < b
now, it will be the next time a and b are compared, and that if a < b then
b < a, and if a == b then b == a.) Some qsort() implementations fail
catastrophically if passed an unstable sort predicate.
***/
void dynlist_qsort(dynlist_t * const p,
 int(*cmp)(const void * const, const void * const)) {
   TSTA(!p); TSTA(!cmp);
   TSTA(p->len < 0); TSTA(p->max < 0);
   TSTA(p->len > p->max);

   if(p->len == 0) return;
   qsort(p->lst, p->len, p->size, cmp);
}

/*** dynlist_dup() -- create a new copy of an existing dynlist

Makes a new dynamic copy of an existing dynlist. The list data is also
copied. Note, however, that if the list data contains pointers, these
will have the same value (thus point to the same underlying data) in both
the old and the new list.

Returns a pointer to the new list on success, NULL on error.
***/
dynlist_t *dynlist_dup(const dynlist_t * const p) {
   dynlist_t *new;

   TSTA(!p); TSTA(p->size > 0); TSTA(p->delta > 0);

   if((new = malloc(sizeof(*new))) == NULL)
      return sysdiagp("malloc", "can't allocate new list");
   memcpy(new, p, sizeof(*new));
   if(new->lst) {
      if((new->lst = malloc(new->size * new->max)) == NULL)
         return sysdiagp("realloc", "can't allocate list data");
      memcpy(new->lst, p->lst, new->size * new->max);
   }
   return new;
}

/*** dynlist_filter() -- remove selected item(s) from a dynlist_t

Removes items from a dynlist_t list, as selected by a callback function.
Arguments:
   p -- pointer to list to be processed
   cmp -- callback to test each individual item
   userdata -- pointer to arbitrary data to be passed to callback

The callback function takes the following arguments:
   p -- pointer to item being considered
   userdata -- same as the userdata pointer passed to dynlist_filter()

The callback should return as follows:
   0 -- keep the item under consideration
   1 -- remove this item from the list
   >1 -- remove this item from the list, and stop processing
   <0 -- keep this item, and stop processing

Returns the number of items removed.
***/
int dynlist_filter(dynlist_t * const p, int(*cmp)(const void * const,
 void * const), void * const userdata) {
   int i, j, n, rtn = 0;
   char *cp;

   TSTA(!p); TSTA(!cmp);

   cp = (char *)(p->lst);
   for(n = i = j = 0; i < p->len; i++) {
      if(rtn == 0 || rtn == 1) {
         if((rtn = cmp(cp+i*p->size, userdata)) > 0) { n++; continue; }
      }
      if(i != j) memcpy(cp+j*p->size, cp+i*p->size, p->size);
      j++;
   }
   p->len = j;

   return n;
}
