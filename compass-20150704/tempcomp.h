#ifndef TEMPCOMP_H
#define TEMPCOMP_H

#include <hmc5883l/hmc5883l.h>

void tempcomp(hmc5883l_pos_t * const pos, const hmc5883l_pos_t * const cur,
 const hmc5883l_pos_t * const orig);

#endif /* ifndef TEMPCOMP_H */
