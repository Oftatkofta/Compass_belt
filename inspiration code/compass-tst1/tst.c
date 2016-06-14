/* This is free software. See COPYING for details. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "dynlist.h"
#include "diag.h"
#include "debug.h"
#include "dataset.h"
#include "rotate.h"

static int vec_add(const int i, pos_t * const pos, void * const userdata) {
   pos_t *sum = (pos_t *)userdata;

   TSTA(!pos); TSTA(!userdata);
   sum->x += pos->x; sum->y += pos->y; sum->z += pos->z;
   return 0;
}

static int find_avg(const dynlist_t * const lst, pos_t * const avg) {
   TSTA(!lst); TSTA(!avg);

   if(lst->len == 0) return diag("can't find average for empty list");

   avg->x = avg->y = avg->z = 0.0;

   if(dataset_foreach((dynlist_t *)lst, vec_add, avg)) return -1;

   avg->x /= (double)(lst->len);
   avg->y /= (double)(lst->len);
   avg->z /= (double)(lst->len);
   return 0;
}

static int xlate(const int i, pos_t * const pos, void * const userdata) {
   pos_t *by = (pos_t *)userdata;
   pos->x -= by->x; pos->y -= by->y; pos->z -= by->z;
   return 0;
}

static int translate(dynlist_t * const lst, const pos_t * const by) {
   return dataset_foreach(lst, xlate, (pos_t *)by);
}

typedef struct {
   int indx_of_max, indx_of_eq;
   double maxdist_sq, eq_best;
   pos_t origin, max, eq;
} dist_t;

static int findmax(const int i, pos_t * const pos, void * const userdata) {
   dist_t *p = (dist_t *)userdata;
   double new, diff;

   diff = pos->x - p->origin.x; new = diff*diff;
   diff = pos->y - p->origin.y; new += diff*diff;
   diff = pos->z - p->origin.z; new += diff*diff;

   if(new <= p->maxdist_sq) return 0;
   p->max.x = pos->x; p->max.y = pos->y; p->max.z = pos->z;
   p->maxdist_sq = new;
   p->indx_of_max = i;
   return 0;
}

static int maxdist(const dynlist_t * const lst, dist_t * const p) {
   p->max.x = p->origin.x; p->max.y = p->origin.y; p->max.z = p->origin.z;
   p->maxdist_sq = 0.0;
   p->indx_of_max = -1;
   return dataset_foreach((dynlist_t *)lst, findmax, p);
}

static int findeq(const int i, pos_t * const pos, void * const userdata) {
   dist_t *p = (dist_t *)userdata;
   double diff, to_n, to_s;

   if(i <= p->indx_of_max) return 0;

   diff = pos->x - p->origin.x; to_n = diff*diff;
   diff = pos->y - p->origin.y; to_n += diff*diff;
   diff = pos->z - p->origin.z; to_n += diff*diff;

   diff = pos->x - p->max.x; to_s = diff*diff;
   diff = pos->y - p->max.y; to_s += diff*diff;
   diff = pos->z - p->max.z; to_s += diff*diff;

   diff = fabs(to_n - to_s);
   if(p->indx_of_eq >= 0 && diff >= p->eq_best) return 0;
   p->eq_best = diff;
   p->indx_of_eq = i;
   p->eq.x = pos->x; p->eq.y = pos->y; p->eq.z = pos->z;
   return 0;
}

static int eqdist(const dynlist_t * const lst, dist_t * const p) {
   p->eq_best = 0.0;
   p->indx_of_eq = -1;
   return dataset_foreach((dynlist_t *)lst, findeq, p);
}

int main(void) {
   pos_t avg, *p;
   dynlist_t *lst;
   dist_t dist;
   rotation_t R1, R2, R3;

   if((lst = dataset_read("circle")) == NULL) return -1;

   if(find_avg(lst, &avg)) return -1;
   diag("C=%lf %lf %lf", avg.x, avg.y, avg.z);

   if(translate(lst, &avg)) return -1;
   diag("N=%lf %lf %lf (1 of %d)", avg.x, avg.y, avg.z, lst->len);

   p = (pos_t *)(lst->lst);
   dist.origin.x = p->x; dist.origin.y = p->y; dist.origin.z = p->z;
   if(maxdist(lst, &dist)) return -1;

   diag("S=%lf %lf %lf (%d of %d)", dist.max.x, dist.max.y, dist.max.z,
    dist.indx_of_max+1, lst->len);

   if(eqdist(lst, &dist)) return -1;

   diag("W=%lf %lf %lf (%d of %d)", dist.eq.x, dist.eq.y, dist.eq.z,
    dist.indx_of_eq+1, lst->len);

   if(rot_find_Rz(&(dist.origin), &R1)) return -1;
   rot_dump(&R1, "1");
   if(rot_set(lst, &R1)) return -1;
   rot_posn(0, &(dist.origin), &R1);
   rot_posn(0, &(dist.eq), &R1);

   diag("N1=%lf %lf %lf", dist.origin.x, dist.origin.y, dist.origin.z);
   diag("W1=%lf %lf %lf", dist.eq.x, dist.eq.y, dist.eq.z);
   //dataset_dump(lst);

   if(rot_find_Rx(&(dist.origin), &R2)) return -1;
   rot_dump(&R2, "2");
   if(rot_set(lst, &R2)) return -1;
   rot_posn(0, &(dist.origin), &R2);
   rot_posn(0, &(dist.eq), &R2);

   diag("N2=%lf %lf %lf", dist.origin.x, dist.origin.y, dist.origin.z);
   diag("W2=%lf %lf %lf", dist.eq.x, dist.eq.y, dist.eq.z);

   if(rot_find_Ry(&(dist.eq), &R3)) return -1;
   rot_dump(&R3, "3");
   if(rot_set(lst, &R3)) return -1;
   rot_posn(0, &(dist.origin), &R3);
   rot_posn(0, &(dist.eq), &R3);

   diag("N3=%lf %lf %lf", dist.origin.x, dist.origin.y, dist.origin.z);
   diag("W3=%lf %lf %lf", dist.eq.x, dist.eq.y, dist.eq.z);

   //dataset_dump(lst);

   return 0;
}
