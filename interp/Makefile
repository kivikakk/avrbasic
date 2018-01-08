TARGET = ../libinterp.a
SRCS = $(wildcard at_*.c)
OBJS = $(SRCS:.c=.o)

include ../Makefile.common

all: run-test

.PHONY: local
local: $(SRCS) local.c
	gcc -DTEST -g -o $@ $^ -Wall
	./local

.PHONY: run-test
run-test: test
	./test

test: $(SRCS) test.c harness.c
	gcc -DTEST -g -o $@ $^ -Wall

.PRECIOUS: $(TARGET)

$(TARGET): $(OBJS)
	avr-ar cru $@ $^ && avr-ranlib $@

clean:
	-rm $(TARGET) $(OBJS) test

docker:
	docker build -t avrbasic-interp $(CURDIR)
	docker run --privileged -t -i -v $(CURDIR)/../:/src/avrbasic/ -w /src/avrbasic/interp avrbasic-interp /bin/bash

docker-valgrind:
	docker build -t avrbasic-interp $(CURDIR)
	docker run --privileged -t -i -v $(CURDIR)/../:/src/avrbasic/ -w /src/avrbasic/interp avrbasic-interp /bin/bash -c 'rm test && make test && valgrind ./test'
