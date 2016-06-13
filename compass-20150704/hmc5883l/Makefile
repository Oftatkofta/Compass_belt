include ../make.vars

CFLAGS+=-I..
SRC=hmc5883l.c

hmc5883l.a: $(SRC:.c=.o)
	avr-ar -r $@ $^

include ../make.rules
