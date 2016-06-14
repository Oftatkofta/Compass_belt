CFLAGS+=-O3 -Wall -Werror -DPARANOIA_LEVEL=PARANOIA_UTMOST -ffast-math
LDLIBS+=-lm
#LDFLAGS+=-g
MAKEDEP=$(CC) $(CFLAGS) -MM
SRC=diag.c dynlist.c fgetrec.c dataset.c rotate.c tst.c
TARGET=tst

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
