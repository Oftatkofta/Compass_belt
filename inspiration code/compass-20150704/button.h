/* Copyright (c) 2014-2015 Douglas Henke <dhenke@mythopoeic.org>
   This is free software -- see COPYING for details.           */
#ifndef BUTTON_H
#define BUTTON_H

#include <avr/pgmspace.h>

#define BTN_DDR DDRB
#define BTN_DDRBIT (1<<DDB1)
#define BTN_PORT PORTB
#define BTN_PORTBIT (1<<PORTB1)
#define BTN_PIN PINB
#define BTN_PINBIT (1<<PINB1)

#define BTN_PRESSED ((BTN_PIN & BTN_PINBIT) == 0)

int button(void);
int button_sleep(const int ms);

#endif /* ifndef BUTTON_H */
