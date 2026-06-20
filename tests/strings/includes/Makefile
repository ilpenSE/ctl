CC = gcc
CFLAGS = -Wall -Wextra -ggdb -I../../src/

BUILD = build
TARGET = $(BUILD)/app
SRCS := $(shell find . -name '*.c')
HEADERS := $(shell find . -name '*.h')
OBJS := $(patsubst %.c, $(BUILD)/%.o, $(SRCS))

DEPENDECIES = ../../src/str.h \
              ../../src/sv.h

all: $(BUILD) $(TARGET)

$(BUILD):
	mkdir -p $(BUILD)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/%.o: %.c $(HEADERS) $(DEPENDECIES) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
