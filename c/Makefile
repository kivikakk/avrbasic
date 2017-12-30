target=	arg
firm_src = ${wildcard at*${target}.c}
firm_obj = ${firm_src:.c=.o}
add_src = ${wildcard u8g2*.c} ${wildcard u8x8*.c}
add_obj = ${add_src:.c=.o}

IPATH = .
IPATH += /usr/local/include/simavr

all: ../libu8g2.a

../libu8g2.a: obj
	$(AVR)$(AR) cru $@ ${firm_obj} ${add_obj} && $(AVR)$(RANLIB) $@

obj: ${firm_obj} ${add_obj}

include Makefile.common

${firm_obj}: 

clean:
	rm -rf *.a *.axf *.o
