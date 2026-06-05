CC = clang
CFLAGS = -Wall -Wextra -ggdb -fsanitize=memory -I../../ -DJSON_IMPLEMENTATION -DEITHER_IMPLEMENTATION

app: main.c
	$(CC) main.c $(CFLAGS) -o app
