#include <stdio.h>
#include <array.h>
#include "utils.h"

int test_ints() {
  Array(int) vec = {0};
  if (!arr_push(&vec, 10)) give_error("arr_push 10");
  if (!arr_push(&vec, 20)) give_error("arr_push 20");
  if (!arr_push(&vec, 30)) give_error("arr_push 30");
  if (!arr_push(&vec, 40)) give_error("arr_push 40");
  print_int_array(&vec);

  TEST_SEPERATOR;

  if (!arr_push(&vec, 69, 67)) give_error("arr_push_many 69, 67");
  print_int_array(&vec);

  TEST_SEPERATOR;

#if 1
  int last_elem = 0;
  if (!arr_pop(&vec, &last_elem)) give_error("arr_pop");
  printf("Removed element was %d:\n", last_elem);
#else
  if (!arr_pop(&vec, NULL)) give_error("arr_pop");
#endif
  print_int_array(&vec);

  TEST_SEPERATOR;

  printf("elem in idx 1: %d\n", *arr_at(&vec, 1));
  printf("last elem: %d\n", *arr_last(&vec));

  TEST_SEPERATOR;

  printf("foreach: [");
  arr_foreach(vec, it) {
    printf("%d, ", *it);
  }
  printf("]\n");

  TEST_SEPERATOR;

  printf(arr_islast(&vec, 69) ? "69 IS LAST!\n" : "69 no last\n");
  printf(arr_isfirst(&vec, 10) ? "10 IS FIRST!\n" : "10 no first\n");

  TEST_SEPERATOR;

  printf("arr_find: %d\n", arr_find(&vec, 20));
  printf("contains 620?: %s\n", arr_contains(&vec, 620) ? "YES" : "NO");

  TEST_SEPERATOR;

  // remove elem with idx 1 (20 in our case)
  if (!arr_remove_unord(&vec, 1)) give_error("arr_remove_unord");
  print_int_array(&vec);

  TEST_SEPERATOR;

  Array(int) vec2 = {0};
  arr_push(&vec2, 0, 1, 2, 3);
  printf("Array2 (gonna be merged into vec) = ");
  print_int_array(&vec2);
  arr_merge(&vec, &vec2);
  printf("Merged array = ");
  print_int_array(&vec);

  TEST_SEPERATOR;

  arr_free(&vec);
  print_int_array(&vec);
  return 0;
}
