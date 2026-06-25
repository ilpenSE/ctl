#pragma once
#include <basic.h>

DECL_ARRAY(int);

int test_strings();
int test_ints();

// Prints int array
void print_int_array(Array(int)* v);

// Prints char* array
void print_cstr_array(Array(char_ptr)* v);

#define give_error(fmt, ...) \
  do { \
    fprintf(stderr, "%s:%d: ERROR: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
  } while (0)

#define TEST_SEPERATOR printf("------------------------------\n");
