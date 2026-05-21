#include <stdio.h>
#include <vector.h>
#include "utils.h"

int test_ints() {
  Vector(int) vec = {0};
  if (!vec_push(&vec, 10)) give_error("vec_push 10");
  if (!vec_push(&vec, 20)) give_error("vec_push 20");
  if (!vec_push(&vec, 30)) give_error("vec_push 30");
  if (!vec_push(&vec, 40)) give_error("vec_push 40");
  print_int_vector(&vec);

  TEST_SEPERATOR;

  if (!vec_push_many(&vec, 69, 67)) give_error("vec_push_many 69, 67");
  print_int_vector(&vec);

  TEST_SEPERATOR;

#if 1
  int last_elem = 0;
  if (!vec_pop(&vec, &last_elem)) give_error("vec_pop");
  printf("Removed element was %d:\n", last_elem);
#else
  if (!vec_pop(&vec, NULL)) give_error("vec_pop");
#endif
  print_int_vector(&vec);

  TEST_SEPERATOR;

  printf("elem in idx 1: %d\n", *vec_at(&vec, 1));
  printf("last elem: %d\n", *vec_last(&vec));

  TEST_SEPERATOR;

  printf("foreach: [");
  vec_foreach(&vec, it) {
    printf("%d, ", *it);
  }
  printf("]\n");

  TEST_SEPERATOR;

  printf(vec_islast(&vec, 69) ? "69 IS LAST!\n" : "69 no last\n");
  printf(vec_isfirst(&vec, 10) ? "10 IS FIRST!\n" : "10 no first\n");

  TEST_SEPERATOR;

  printf("vec_find: %d\n", vec_find(&vec, 20));
  printf("contains 620?: %s\n", vec_contains(&vec, 620) ? "YES" : "NO");

  TEST_SEPERATOR;

  // remove elem with idx 1 (20 in our case)
  if (!vec_remove_unord(&vec, 1)) give_error("vec_remove_unord");
  print_int_vector(&vec);

  TEST_SEPERATOR;

  vec_free(&vec);
  print_int_vector(&vec);
  return 0;
}
