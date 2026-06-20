CC = clang
CFLAGS = -Wall -Wextra -Wno-initializer-overrides -Wno-override-init -ggdb -fsanitize=memory -I../../src/ -DJSON_IMPLEMENTATION -DEITHER_IMPLEMENTATION -DFUTIL_IMPLEMENTATION -DSTR_IMPLEMENTATION -DSV_IMPLEMENTATION -DARRAY_IMPLEMENTATION -DBASIC_IMPLEMENTATION

app: main.c
	$(CC) main.c $(CFLAGS) -o app
