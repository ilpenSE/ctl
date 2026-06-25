CC = clang
CFLAGS = -Wall -Wextra -I../../include/

BUILD = build
TARGET = $(BUILD)/app
SRC = main.c ints.c strings.c
DEPS_H = \
  ../../include/array.h \
  ../../include/basic.h

DEPS_C = \
  ../../src/array.c \
  ../../src/basic.c

DEPS = $(DEPS_C) $(DEPS_H)

EMPTY_TARGET = $(BUILD)/empty
EMPTY_SRC = empty.c

ifeq ($(debug),1)
	CFLAGS += -ggdb -O0 -fsanitize=memory
else
	CFLAGS += -O2
endif

all: $(TARGET) $(EMPTY_TARGET) $(BUILD)

$(BUILD):
	mkdir -p $(BUILD)

$(EMPTY_TARGET): $(EMPTY_C) $(DEPS) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $(EMPTY_SRC) $(DEPS_C)

$(TARGET): $(SRC) $(DEPS) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(DEPS_C)

clean:
	rm -rf $(BUILD)

.PHONY: all clean
