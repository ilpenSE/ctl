CC = clang
CFLAGS = -Wall -Wextra -ggdb -fsanitize=memory -I../../src/

TARGET = app
SRC = main.c

$(TARGET):
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)
