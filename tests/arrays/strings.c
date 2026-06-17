#include <stdio.h>
#include <string.h>
#include <array.h>
#include "utils.h"

int test_strings() {
  Array(char_ptr) vec = {0};
  if (!arr_push(&vec, "hello")) give_error("Failed to push");
  if (!arr_push(&vec, "world")) give_error("Failed to push");
  print_cstr_array(&vec);

  TEST_SEPERATOR;

  printf("idx of hello: %d\n", arr_find(&vec, "world"));

  TEST_SEPERATOR;

  if (!arr_push(&vec, "selamın", "aleyküm", "aleyküm", NULL))
    give_error("Failed to push many");
  print_cstr_array(&vec);
  printf("idx of aleyküm: %d\n", arr_find(&vec, "aleyküm")); // will print first element's idx

  TEST_SEPERATOR;

  size_t idx = 2;
  if (!arr_remove_idx(&vec, idx))
    give_error("Failed to remove idx: %zu", idx);
  print_cstr_array(&vec);

  TEST_SEPERATOR;

  arr_free(&vec);
  printf("Printing vec after free:\n");
  print_cstr_array(&vec);
  // You cannot use vec after free
  // Simply, functions return falsy values (false, -1 etc.)
  // And you cannot iterate it. If you do, program'll be aborted with assertion failure
  printf("Is vec freed: %s\n", arr_isfreed(&vec) ? "YES" : "NO"); // YES
#if 1
  char** s = arr_at(&vec, 0); // ptr to the element (can be NULL if vec is freed or out-of-bounds)
  if (s) printf("%s\n", *s);
  else printf("(null)\n"); // this'll be printed
#else
  if (!arr_push(&vec, "sa")) give_error("arr_push");
  printf("Is vec freed: %s\n", arr_isfreed(&vec) ? "YES" : "NO"); // YES
#endif

  TEST_SEPERATOR;

  Array(char_ptr) vec1 = {0};
  arr_push(&vec1, "hello", "world", "!");
  printf("Vec1 = ");
  print_cstr_array(&vec1);

  Array(char_ptr) vec2 = {0};
  arr_push(&vec2, "hello", "world", "!");
  printf("Vec2 = ");
  print_cstr_array(&vec2);

  printf("Are they equal: %s\n", arr_equals(&vec1, &vec2) ? "YES" : "NO");
  return 0;
}
