TARGET = ../libinterp.a
SRCS = $(wildcard at_*.c)
OBJS = $(SRCS:.c=.o)

include ../Makefile.common

all: test

.PHONY: test
test:
	gcc -o $@ $(SRCS) test.c harness.c -Wall
	./$@

.PRECIOUS: $(TARGET)

$(TARGET): $(OBJS)
	avr-ar cru $@ $^ && avr-ranlib $@
