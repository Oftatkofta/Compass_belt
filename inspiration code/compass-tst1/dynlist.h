#ifndef DYNLIST_H
#define DYNLIST_H

typedef struct {
   int max, len, size, delta;
   void *lst;
} dynlist_t;

dynlist_t *dynlist_new(const int size, const int delta);
int dynlist_free(dynlist_t * const p);
int dynlist_add(dynlist_t * const p, const void * const item);
void dynlist_qsort(dynlist_t * const p,
 int(*cmp)(const void * const, const void * const));
dynlist_t *dynlist_dup(const dynlist_t * const p);
int dynlist_filter(dynlist_t * const p, int(*cmp)(const void * const,
 void * const), void * const userdata);

#endif /* ifndef DYNLIST_H */
