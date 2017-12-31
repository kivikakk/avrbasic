TARGET = target/avr-atmega328p/release/avrbasic.elf

all:
	./build.sh

dump: all
	avr-objdump -d $(TARGET)

simavr: all
	simavr $(TARGET) -m atmega328p -g

gdb:
	avr-gdb $(TARGET) -ex 'target remote localhost:1234'

flash: all
	avr-objcopy -j .text -j .data -j .eeprom -O ihex $(TARGET) $(TARGET:.elf=.hex)
	avrdude -c usbtiny -p atmega328p -U flash:w:$(TARGET:.elf=.hex) -B 1

docker:
	docker build -t avrbasic  $(CURDIR)
	docker run --privileged -t -i -v $(CURDIR):/src/avrbasic -v $(HOME)/.cargo/registry:/root/.cargo/registry -w /src/avrbasic avrbasic /bin/bash
