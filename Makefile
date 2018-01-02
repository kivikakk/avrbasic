TARGET = avrbasic.elf
firm_src = ${wildcard at_*.c}
firm_obj = ${firm_src:.c=.o}
add_src = ${wildcard u8g2*.c} ${wildcard u8x8*.c}
add_obj = ${add_src:.c=.o}

IPATH = .
IPATH += /usr/local/include/simavr

include Makefile.common

all: $(TARGET)

$(TARGET): ${firm_obj} ${add_obj} libinterp.a
	@avr-gcc -Wall -gdwarf-2 -Os -std=gnu99 \
			-mmcu=atmega328 \
			-DF_CPU=8000000 \
			-fno-inline-small-functions \
			-ffunction-sections -fdata-sections \
			-Wl,--relax,--gc-sections \
			-Wl,--undefined=_mmcu,--section-start=.mmcu=0x910000 \
			${CPPFLAGS} \
			$^ -o $@
	@avr-size $@|sed '1d'

libinterp.a: $(wildcard interp/*.[ch])
	$(MAKE) -C interp

dump: all
	avr-objdump -d $(TARGET)

simavr: all
	simavr $(TARGET) -m atmega328p -g

gdb:
	avr-gdb $(TARGET) -ex 'target remote localhost:1234'

flash: all
	avr-objcopy -j .text -j .data -j .eeprom -O ihex $(TARGET) $(TARGET:.elf=.hex)
	avrdude -c usbtiny -p atmega328p -U flash:w:$(TARGET:.elf=.hex) -B 1

clean:
	rm -rf *.a *.axf *.o *.hex
