CFLAGS=-Wall -Wextra -g -I../include -DHAS_STDINT_H
LDFLAGS=-L. -lfiber-framework -Wl,-rpath,\$$ORIGIN
CC=gcc

SRC_DIR=.

TESTS_SRCS= \
	$(SRC_DIR)/tests.c

default: all

all: ff-tests

libfiber-framework.so:
	cd .. && make libfiber-framework.so && cp libfiber-framework.so ./tests/

ff-tests: libfiber-framework.so $(TESTS_SRCS)
	$(CC) $(CFLAGS) -o ff-tests $(TESTS_SRCS) $(LDFLAGS)

clean:
	rm -f libfiber-framework.so ff-tests

