TARGET = ../libinterp.a
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

include ../Makefile.common

all: $(TARGET)

.PRECIOUS: $(TARGET)

$(TARGET): $(OBJS)
	avr-ar cru $@ $^ && avr-ranlib $@
