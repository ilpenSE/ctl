#include <stdio.h>
#include <string.h>
#define VECTOR_IMPLEMENTATION
#include <vector.h>
#include "utils.h"

int main(int argc, char** argv) {
  const char* kind = "";
  if (argc >= 2) kind = argv[1];

  if (strncmp(kind, "ints", 4) == 0) return test_ints();
  if (strncmp(kind, "strings", 7) == 0) return test_strings();

  printf("Unknown test kind.\n");
  printf("Usage:\n");
  printf("  %s ints   : Test int vectors\n", argv[0]);
  printf("  %s strings: Test string (char*) vectors\n", argv[0]);
  return 1;
}

void print_int_vector(Vector(int)* v) {
  size_t len = vec_len(v);
  printf(STRINGIFY(Vector(int)) "{items: [");
  if (v->items) {
    for (size_t i = 0; i < len; i++) {
      const char* suffix = i == len - 1 ? "" : ", ";
      int* elem = vec_at(v, i);
      if (elem) printf("%d%s", *elem, suffix);
      else printf("(null)%s", suffix);
    }
  }
  printf("], ");
  printf("capacity: %zu, ", vec_cap(v));
  printf("length: %zu}\n", len);
}

void print_cstr_vector(Vector(char_ptr)* v) {
  size_t len = vec_len(v);
  printf(STRINGIFY(Vector(char_ptr)) "{items: [");
  if (v->items) {
    for (size_t i = 0; i < len; i++) {
      const char* suffix = i == len - 1 ? "" : ", ";
      char** elem = vec_at(v, i);
      if (elem) printf("\"%s\"%s", *elem, suffix);
      else printf("\"(null)\"%s", suffix);
    }
  }
  printf("], ");
  printf("capacity: %zu, ", vec_cap(v));
  printf("length: %zu}\n", len);
}
