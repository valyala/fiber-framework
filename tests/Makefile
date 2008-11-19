CFLAGS=-Wall -fpic -combine -fwhole-program -g -I../include -DHAS_STDINT_H -U_FORTIFY_SOURCE
LDFLAGS=-lfiber-framework
CC=gcc

SRC_DIR=.

TESTS_SRCS= \
	$(SRC_DIR)/tests.c

default: all

all: ff-tests

libfiber-framework.so:
	cd .. && make libfiber-framework.so && cp libfiber-framework.so ./tests

ff-tests: libfiber-framework.so $(TESTS_SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -L. -Wl,--rpath -Wl,. -o ff-tests $(TESTS_SRCS)

clean:
	rm -f libfiber-framework.so ff-tests

