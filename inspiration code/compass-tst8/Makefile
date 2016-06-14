 CFLAGS+=-O3 -Wall -Werror -DPARANOIA_LEVEL=PARANOIA_UTMOST
MAKEDEP=$(CC) $(CFLAGS) -MM
LDLIBS+=-lm
SRC=diag.c i2c_ops.c hmc5883l.c main.c rotate.c calibrate.c heading.c fixedpt.c
TARGET=compass-read

all: $(TARGET)

clean:
	rm -f $(TARGET) *.[oad] core

$(TARGET): $(SRC:.c=.o)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.d:%.c
	$(MAKEDEP) $< >$@

ifneq ($(MAKECMDGOALS),clean)
include $(SRC:.c=.d) # always include dependency files for all .c files
endif
