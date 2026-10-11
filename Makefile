
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -Iinclude
BUILD_DIR = build
TARGET = $(BUILD_DIR)/os_toolkit

SOURCES = src/main.c \
          src/scheduling/scheduling.c

all: $(TARGET)

$(TARGET): $(SOURCES) include/scheduling.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
