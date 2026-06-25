CC = clang
CFLAGS = -Wall -Wextra -Wno-initializer-overrides -Wno-override-init -ggdb -fsanitize=memory -I../../

app: main.c
	$(CC) main.c $(CFLAGS) -o app
