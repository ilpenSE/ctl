CC = clang
CFLAGS = -Wall -Wextra -ggdb -fsanitize=memory -I../../src/ -DFUTIL_IMPLEMENTATION -DSTR_IMPLEMENTATION -DEITHER_IMPLEMENTATION

BUILD = build
TARGET = $(BUILD)/app
SRC = directories.c
# SRC = main.c

all: $(TARGET) $(BUILD)

$(BUILD):
	mkdir -p $(BUILD)

$(TARGET): $(SRC) | $(BUILD)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
