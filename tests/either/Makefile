CC = clang
CFLAGS = -Wall -Wextra -ggdb -fsanitize=memory -I../../include/

TARGET = app
SRC = main.c

$(TARGET):
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) ../../src/either.c
