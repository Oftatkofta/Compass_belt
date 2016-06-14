include ../make.vars

CFLAGS+=-I..
SRC=matrix8x8.c

matrix8x8.a: $(SRC:.c=.o)
	avr-ar -r $@ $^

include ../make.rules
