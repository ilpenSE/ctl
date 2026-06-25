CC = clang
CFLAGS = -Wall -Wextra -Wno-initializer-overrides -ggdb -fsanitize=memory -I../../

BUILD = build
TARGET = $(BUILD)/app
SRC = directories.c
SRC = main.c

all: $(TARGET) $(BUILD)

$(BUILD):
	mkdir -p $(BUILD)

$(TARGET): $(SRC) | $(BUILD)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
