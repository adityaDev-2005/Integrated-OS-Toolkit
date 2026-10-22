CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -pthread -Iinclude
BUILD_DIR = build
TARGET = $(BUILD_DIR)/os_toolkit
SOURCES = src/main.c \
          src/benchmark/benchmark.c \
          src/benchmark/csv_logger.c \
          src/common/common.c \
          src/deadlock/banker.c \
          src/disk/disk.c \
          src/file_alloc/file_alloc.c \
          src/memory/memory.c \
          src/paging/paging.c \
          src/posix/process_demo.c \
          src/posix/producer_consumer.c \
          src/scheduling/scheduling.c
HEADERS = include/banker.h \
          include/benchmark.h \
          include/common.h \
          include/disk.h \
          include/file_alloc.h \
          include/memory.h \
          include/paging.h \
          include/process_demo.h \
          include/producer_consumer.h \
          include/scheduling.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

test: $(TARGET)
	bash tests/run_tests.sh

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test clean
