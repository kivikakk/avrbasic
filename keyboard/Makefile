TARGET = keyboard.elf
firm_src = ${wildcard at_*.c}
firm_obj = ${firm_src:.c=.o}

IPATH = .
IPATH += /usr/local/include/simavr

include Makefile.common

all: $(TARGET)

$(TARGET): ${firm_obj}
	@avr-gcc -Wall -gdwarf-2 -Os -std=gnu99 \
			-mmcu=atmega8 \
			-DF_CPU=8000000 \
			-fno-inline-small-functions \
			-ffunction-sections -fdata-sections \
			-Wl,--relax,--gc-sections \
			-Wl,--undefined=_mmcu,--section-start=.mmcu=0x910000 \
			${CPPFLAGS} \
			$^ -o $@
	@avr-size $@|sed '1d'

dump: all
	avr-objdump -d $(TARGET)

simavr: all
	simavr $(TARGET) -m atmega328p -g

gdb:
	avr-gdb $(TARGET) -ex 'target remote localhost:1234'

flash: all
	avr-objcopy -j .text -j .data -j .eeprom -O ihex $(TARGET) $(TARGET:.elf=.hex)
	avrdude -c usbtiny -p atmega8 -U flash:w:$(TARGET:.elf=.hex) -B 1

clean:
	rm -rf *.a *.axf *.o *.hex
