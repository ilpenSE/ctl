CC = clang
CFLAGS = -Wall -Wextra -I../../include

BASIC_H = ../../include/basic.h
BASIC_C = ../../src/basic.c

app: main.c $(BASIC_H) $(BASIC_C) Makefile
	$(CC) $(CFLAGS) -o $@ $< $(BASIC_C)
