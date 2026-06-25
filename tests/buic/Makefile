CC = clang
CFLAGS = -Wall -Wextra -Wno-initializer-overrides -ggdb -I../../build/include/ -lm

buic: build.c ../../src/buic.c
	$(CC) $(CFLAGS) build.c -o buic
