CC = clang
BUILD = ../../.build
CFLAGS = -Wall -Wextra -ggdb -I$(BUILD)/include
# FUCK GNU LINKER
LDFLAGS = -L$(BUILD)/lib -Wl,-rpath,$(BUILD)/lib
LD_LIBS = -lctl
SRC = main.c
TARGET = app

$(TARGET): $(SRC) Makefile
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) $(LD_LIBS) -o $(TARGET)
