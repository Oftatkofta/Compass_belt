include make.vars

#ISP=avrdude -c usbtiny -p $(MCU)
ISP=avrdude -c usbtiny -p trinket
CFLAGS+=-I.

TARGET=compass
SRC=button.c rotate.c calibrate.c compass.c heading.c fixedpt.c stored_cal.c \
 tempcomp.c validate.c

.PHONY: all program sizeprof getfuse clean

all: $(TARGET).hex

program : $(TARGET).hex
	$(ISP) -e -U flash:w:$(TARGET).hex

sizeprof: map
	@chmod a+x ./prof-size
	./prof-size $^

getfuse:
	$(ISP) -U hfuse:r:fuse-h.txt:b -U lfuse:r:fuse-l.txt:b \
         -U efuse:r:fuse-e.txt:b

$(TARGET).elf map: $(SRC:.c=.o) uWireM/uWireM.a matrix8x8/matrix8x8.a \
 hmc5883l/hmc5883l.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

uWireM/uWireM.a: uWireM/*.[ch]
	make -C uWireM

matrix8x8/matrix8x8.a: matrix8x8/*.[ch]
	make -C matrix8x8

hmc5883l/hmc5883l.a: hmc5883l/*.[ch]
	make -C hmc5883l

%.hex : %.elf
	avr-objcopy -R .eeprom -O ihex $< $@
	avr-size $<

include make.rules

clean::
	rm -f *.hex *.elf fuse-*.txt $(TARGET) map
	make -C uWireM clean
	make -C matrix8x8 clean
	make -C hmc5883l clean
