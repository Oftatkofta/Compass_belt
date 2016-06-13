/* Copyright (c) 2014-2015 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#ifndef U_WIRE_M_H
#define U_WIRE_M_H

/* See README for introduction and licensing information. */

#include "USI_TWI_Master.h"

/*** uWireM_init() -- initialize USI hardware for use as I2C master

This macro calls an initialization function which sets up the USI
hardware for use as an I2C master. It must be called once before any
I2C data can be sent or received using uWireM_xfer().

It must also be called again after the USI hardware is powered down
and back up, before any further I2C communication can take place.

Takes no arguments; returns void.
***/
#define uWireM_init USI_TWI_Master_Initialise

/*** uWireM_addr_send() -- calculate first transfer byte for I2C send

This macro calculates the value that should be placed in the first byte
of the I2C transfer buffer in order to send data to a specific slave
device.

The argument is the I2C address of the slave device to which data is to
be sent. This should be an integer value in the range 0x10 - 0x77
inclusive. (10-bit addressing is not supported.)

Returns the value to be placed at the start of the transfer buffer
prior to calling uWireM_xfer().
***/
#define uWireM_addr_send(x) ((x)<<TWI_ADR_BITS)

/*** uWireM_addr_recv() -- calculate first transfer byte for I2C receive

Identical to uWireM_addr_send() (which see), except used to request
data from (rather than send data to) a slave device.
***/
#define uWireM_addr_recv(x) (((x)<<TWI_ADR_BITS) | 1)

/*** uWireM_xfer() -- transfer I2C data

Transfers data to or from an I2C slave device.

The first argument is a pointer to the data buffer. For sending, the
first byte of the buffer must be a value calculated using uWireM_addr_send(),
and the following byte(s) must contain the data to be sent. For receiving,
the first byte must be a value calculated using uWireM_addr_recv(); the
following byte(s) are used to store the data read from the slave device.

The second argument is the length of the data to be sent or received,
including the initial address byte. This must be a value in the range
2-255, and the buffer pointed to by the first argument must be of at least
the specified length.

Returns TRUE on success, FALSE on failure. In the event of a failure,
the specific problem can be determined by calling uWireM_err().
***/
#define uWireM_xfer USI_TWI_Start_Transceiver_With_Data

/*** uWireM_err() -- get error code after failed data transfer

This macro can be used to get the error code after a call to uWireM_xfer()
has returned FALSE to indicate failure.

Takes no arguments.

The value returned is one of the USI_TWI_* error codes defined in
USI_TWI_Master.h.
***/
#define uWireM_err USI_TWI_Get_State_Info

#endif /* ifndef U_WIRE_M_H */
