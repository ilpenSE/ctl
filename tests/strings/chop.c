#include <stdio.h>
#include "prettyp.h"

void delim_test(StringView view) {
  printf("view (original) = \""SV_FMT"\"\n", SV_ARG(view));
  StringView chopped = sv_chop_by_delim(&view, ',');
  printf("view (after chop) = \""SV_FMT"\"\n", SV_ARG(view));
  printf("chopped = \""SV_FMT"\"\n", SV_ARG(chopped));
}

#define TEST_CASES \
  X("delim_exists_at_mid", "hello,world") \
  X("delim_exists_at_begin", ",helloworld") \
  X("delim_exists_at_end", "helloworld,") \
  X("delim_double", "hello,,world") \
  X("delim_triple", "hello,,,world") \
  X("delim_not_exists", "helloworld")

int main() {
  #define X(name, cstr) \
    printf("===== "name" begin =====\n"); \
    delim_test(sv((cstr))); \
    printf("===== "name" end =====\n\n");
  TEST_CASES
  #undef X

  StringView sv1 = sv("hello world!");
  print_sv(sv1);

  sv_chop_left(&sv1, 2);
  print_sv(sv1);

  sv_chop_right(&sv1, 2);
  print_sv(sv1);

  return 0;
}
