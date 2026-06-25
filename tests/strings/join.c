#include <stdio.h>
#include "prettyp.h"
#include <ctl.h>

int main() {
  String s = {0};
  Array(cchar_ptr) arr = {0};
  arr_push(&arr, "merhaba");
  arr_push(&arr, "dünya");
  str_join(&s, &arr, ' ');
  printf("s = \"%s\"\n", s.data);
  return 0;
}
