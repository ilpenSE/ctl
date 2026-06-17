CC = clang
CFLAGS = -Wall -Wextra -I../../

BUILD = build
TARGET = $(BUILD)/app
SRC = main.c ints.c strings.c
ARRAY_H = ../../array.h

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

$(EMPTY_TARGET): $(EMPTY_C) $(ARRAY_H) | $(BUILD)
	$(CC) $(CFLAGS) -o $(EMPTY_TARGET) $(EMPTY_SRC)

$(TARGET): $(SRC) $(ARRAY_H) | $(BUILD)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -rf $(BUILD)

.PHONY: all clean
