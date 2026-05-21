#include <stdio.h>
#include <string.h>
#include <vector.h>
#include "utils.h"

int test_strings() {
  Vector(char_ptr) vec = {0};
  if (!vec_push(&vec, "hello")) give_error("Failed to push");
  if (!vec_push(&vec, "world")) give_error("Failed to push");
  print_cstr_vector(&vec);

  TEST_SEPERATOR;

  printf("idx of hello: %d\n", vec_find(&vec, "world"));

  TEST_SEPERATOR;

  if (!vec_push_many(&vec, "selamın", "aleyküm", "aleyküm", NULL))
    give_error("Failed to push many");
  print_cstr_vector(&vec);
  printf("idx of aleyküm: %d\n", vec_find(&vec, "aleyküm")); // will print first element's idx

  TEST_SEPERATOR;

  size_t idx = 2;
  if (!vec_remove_idx(&vec, idx))
    give_error("Failed to remove idx: %zu", idx);
  print_cstr_vector(&vec);

  TEST_SEPERATOR;

  vec_free(&vec);
  printf("Printing vec after free:\n");
  print_cstr_vector(&vec);
  // You cannot use vec after free
  // Simply, functions return falsy values (false, -1 etc.)
  // And you cannot iterate it. If you do, program'll be aborted with assertion failure
  printf("Is vec freed: %s\n", vec_isfreed(&vec) ? "YES" : "NO"); // YES
#if 1
  char** s = vec_at(&vec, 0); // ptr to the element (can be NULL if vec is freed or out-of-bounds)
  if (s) printf("%s\n", *s);
  else printf("(null)\n"); // this'll be printed
#else
  if (!vec_push(&vec, "sa")) give_error("vec_push");
  printf("Is vec freed: %s\n", vec_isfreed(&vec) ? "YES" : "NO"); // YES
#endif

  TEST_SEPERATOR;

  Vector(char_ptr) vec1 = {0};
  vec_push_many(&vec1, "hello", "world", "!");
  printf("Vec1 = ");
  print_cstr_vector(&vec1);

  Vector(char_ptr) vec2 = {0};
  vec_push_many(&vec2, "hello", "world", "!");
  printf("Vec2 = ");
  print_cstr_vector(&vec2);

  printf("Are they equal: %s\n", vec_equals(&vec1, &vec2) ? "YES" : "NO");
  return 0;
}
