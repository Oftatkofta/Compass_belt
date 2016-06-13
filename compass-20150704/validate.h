#ifndef VALIDATE_H
#define VALIDATE_H

#include <hmc5883l/hmc5883l.h>

#define VLD_ERR_OK 0
#define VLD_ERR_TILT 0x01
#define VLD_ERR_INTF 0x02

int validate(const hmc5883l_pos_t * const pos);

#endif /* ifndef VALIDATE_H */
