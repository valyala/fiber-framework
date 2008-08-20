CFLAGS=-Wall -fpic -g -I./include -DHAS_STDINT_H -D_GNU_SOURCE
LDFLAGS=-lpthread -shared
CC=gcc

SRC_DIR=src
ARCH_DIR=$(SRC_DIR)/arch/linux
TESTS_DIR=unit-tests

ARCH_SRCS= \
	$(ARCH_DIR)/ff_arch_completion_port.c \
	$(ARCH_DIR)/ff_arch_fiber.c \
	$(ARCH_DIR)/ff_arch_file.c \
	$(ARCH_DIR)/ff_arch_misc.c \
	$(ARCH_DIR)/ff_arch_mutex.c \
	$(ARCH_DIR)/ff_arch_net_addr.c \
	$(ARCH_DIR)/ff_arch_tcp.c \
	$(ARCH_DIR)/ff_arch_thread.c \
	$(ARCH_DIR)/ff_arch_udp.c \
	$(ARCH_DIR)/ff_linux_error_check.c \
	$(ARCH_DIR)/ff_linux_net.c

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
	$(ARCH_DIR)/ff_linux_error_check.o \
	$(ARCH_DIR)/ff_linux_net.o

MAIN_SRCS= \
	$(SRC_DIR)/ff_blocking_queue.c \
	$(SRC_DIR)/ff_blocking_stack.c \
	$(SRC_DIR)/ff_container.c \
	$(SRC_DIR)/ff_core.c \
	$(SRC_DIR)/ff_dictionary.c \
	$(SRC_DIR)/ff_event.c \
	$(SRC_DIR)/ff_fiber.c \
	$(SRC_DIR)/ff_fiberpool.c \
	$(SRC_DIR)/ff_file.c \
	$(SRC_DIR)/ff_hash.c \
	$(SRC_DIR)/ff_malloc.c \
	$(SRC_DIR)/ff_mutex.c \
	$(SRC_DIR)/ff_pool.c \
	$(SRC_DIR)/ff_queue.c \
	$(SRC_DIR)/ff_read_stream_buffer.c \
	$(SRC_DIR)/ff_semaphore.c \
	$(SRC_DIR)/ff_stack.c \
	$(SRC_DIR)/ff_tcp.c \
	$(SRC_DIR)/ff_threadpool.c \
	$(SRC_DIR)/ff_udp.c \
	$(SRC_DIR)/ff_write_stream_buffer.c

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
	$(SRC_DIR)/ff_malloc.o \
	$(SRC_DIR)/ff_mutex.o \
	$(SRC_DIR)/ff_pool.o \
	$(SRC_DIR)/ff_queue.o \
	$(SRC_DIR)/ff_read_stream_buffer.o \
	$(SRC_DIR)/ff_semaphore.o \
	$(SRC_DIR)/ff_stack.o \
	$(SRC_DIR)/ff_tcp.o \
	$(SRC_DIR)/ff_threadpool.o \
	$(SRC_DIR)/ff_udp.o \
	$(SRC_DIR)/ff_write_stream_buffer.o

FF_LIB_SRCS= \
	$(ARCH_SRCS) \
	$(MAIN_SRCS)

FF_LIB_OBJS= \
	$(ARCH_OBJS) \
	$(MAIN_OBJS)

ALL_SRCS= \
	$(FF_LIB_SRCS)

ALL_OBJS= \
	$(FF_LIB_OBJS)

.c:
	$(CC) $(CFLAGS) $@.c $(LDFLAGS) -o $@

default: all

all: lib-ff.so

lib-ff.so: $(FF_LIB_OBJS)
	$(CC) $(FF_LIB_OBJS) $(LDFLAGS) -o $@

clean:
	rm -f $(ALL_OBJS) lib-ff.so

touch:
	touch $(ALL_SRCS)
	touch include/private/*.h include/private/arch/*.h include/public/*.h include/public/arch/*.h
	touch Makefile
