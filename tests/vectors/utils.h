#pragma once

#define STRINGIFY(x) #x

DECL_VECTOR(int, int);
DECL_VECTOR(char*, char_ptr);

int test_strings();
int test_ints();

// Prints int vector
void print_int_vector(Vector(int)* v);

// Prints char* vector
void print_cstr_vector(Vector(char_ptr)* v);

#define give_error(fmt, ...) \
  do { \
    fprintf(stderr, "%s:%d: ERROR: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
  } while (0)

#define TEST_SEPERATOR printf("------------------------------\n");
