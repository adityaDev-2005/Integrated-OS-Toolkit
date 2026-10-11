
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -Iinclude
BUILD_DIR = build
TARGET = $(BUILD_DIR)/os_toolkit

SOURCES = src/main.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
