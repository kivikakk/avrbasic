TARGET = ../libinterp.a
SRCS = $(wildcard at_*.c)
OBJS = $(SRCS:.c=.o)

include ../Makefile.common

all: test

.PHONY: test
test:
	gcc -g -DTEST -o $@ $(SRCS) test.c harness.c -Wall
	./$@

.PRECIOUS: $(TARGET)

$(TARGET): $(OBJS)
	avr-ar cru $@ $^ && avr-ranlib $@

docker:
	docker build -t avrbasic-interp $(CURDIR)
	docker run --privileged -t -i -v $(CURDIR)/../:/src/avrbasic/ -w /src/avrbasic/interp avrbasic-interp /bin/bash
