#include <stdio.h>
#include "prettyp.h"

int main() {
  String s = str_new("Hello, World!");
  StringView sv1 = sv_from_str(&s, .start = 0, .end = s.len); // Hello, World!
  StringView sv2 = sv_from_str(&s, .start = 0, .end = 4); // Hell
  StringView sv3 = sv_from_str(&s, .start = 0, .end = 50); // empty
  StringView sv4 = sv_from_str(&s, .start = 7, .end = s.len); // World!

  const char* some_str = "Çağatay Akman";
  //                      ^^^ -> 5 bytes
  StringView sv5 = sv_from_cstre(some_str, strlen(some_str), .start = 0, .end = 5); // Çağ

  print_sv(&sv1);
  print_sv(&sv2);
  print_sv(&sv3);
  print_sv(&sv4);
  print_sv(&sv5);

  str_free(&s);
  return 0;
}
