CC = clang
BUILD = ../../build/
CFLAGS = -Wall -Wextra -ggdb -I$(BUILD) -L$(BUILD) -lcutils
SRC = main.c
TARGET = app

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	LD_LIBRARY_PATH=../../build/ ./app
