CFLAGS=-Wall -fpic -fvisibility=hidden -g -I./include -DHAS_STDINT_H -D_GNU_SOURCE
LDFLAGS=-lpthread -shared
CC=gcc

SRC_DIR=src
ARCH_DIR=$(SRC_DIR)/arch/linux
TESTS_DIR=tests

ARCH_OBJS= \
	$(ARCH_DIR)/ff_arch_completion_port.o \
	$(ARCH_DIR)/ff_arch_fiber.o \
	$(ARCH_DIR)/ff_arch_file.o \
	$(ARCH_DIR)/ff_arch_misc.o \
	$(ARCH_DIR)/ff_arch_mutex.o \
	$(ARCH_DIR)/ff_arch_net_addr.o \
	$(ARCH_DIR)/ff_arch_tcp.o \
	$(ARCH_DIR)/ff_arch_thread.o \
	$(ARCH_DIR)/ff_arch_udp.o \
	$(ARCH_DIR)/ff_linux_net.o

MAIN_OBJS= \
	$(SRC_DIR)/ff_blocking_queue.o \
	$(SRC_DIR)/ff_blocking_stack.o \
	$(SRC_DIR)/ff_container.o \
	$(SRC_DIR)/ff_core.o \
	$(SRC_DIR)/ff_dictionary.o \
	$(SRC_DIR)/ff_event.o \
	$(SRC_DIR)/ff_fiber.o \
	$(SRC_DIR)/ff_fiberpool.o \
	$(SRC_DIR)/ff_file.o \
	$(SRC_DIR)/ff_hash.o \
	$(SRC_DIR)/ff_log.o \
	$(SRC_DIR)/ff_malloc.o \
	$(SRC_DIR)/ff_mutex.o \
	$(SRC_DIR)/ff_pool.o \
	$(SRC_DIR)/ff_queue.o \
	$(SRC_DIR)/ff_read_stream_buffer.o \
	$(SRC_DIR)/ff_semaphore.o \
	$(SRC_DIR)/ff_stack.o \
	$(SRC_DIR)/ff_stream.o \
	$(SRC_DIR)/ff_stream_acceptor.o \
	$(SRC_DIR)/ff_stream_acceptor_tcp.o \
	$(SRC_DIR)/ff_stream_connector.o \
	$(SRC_DIR)/ff_stream_connector_tcp.o \
	$(SRC_DIR)/ff_stream_tcp.o \
	$(SRC_DIR)/ff_tcp.o \
	$(SRC_DIR)/ff_threadpool.o \
	$(SRC_DIR)/ff_udp.o \
	$(SRC_DIR)/ff_write_stream_buffer.o

FF_LIB_OBJS= \
	$(ARCH_OBJS) \
	$(MAIN_OBJS)

ALL_OBJS= \
	$(FF_LIB_OBJS)

default: all

all: libfiber-framework.so tests

libfiber-framework.so: $(FF_LIB_OBJS)
	$(CC) $(FF_LIB_OBJS) $(LDFLAGS) -o $@

tests: libfiber-framework.so
	$(CC) -g -I./include -DHAS_STDINT_H -lfiber-framework -L. -Wl,--rpath -Wl,. -o run-tests $(TESTS_DIR)/tests.c

clean:
	rm -f $(ALL_OBJS) libfiber-framework.so run-tests
