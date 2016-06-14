include ../make.vars

SRC=USI_TWI_Master.c

uWireM.a: $(SRC:.c=.o)
	avr-ar -r $@ $^

include ../make.rules
