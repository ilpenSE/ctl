CC = clang
BUILD = ../../build
CFLAGS = -Wall -Wextra -ggdb -I$(BUILD)/include
# FUCK GNU LINKER
LDFLAGS = -L$(BUILD)/lib -Wl,-rpath,$(BUILD)/lib
LD_LIBS = -lcutils
SRC = main.c
TARGET = app

$(TARGET): $(SRC) Makefile
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS) $(LD_LIBS)
