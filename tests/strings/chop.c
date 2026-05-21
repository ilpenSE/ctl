#include <stdio.h>
#include "prettyp.h"

int main() {
  StringView sv1 = sv_from_cstr("hello world!");
  print_sv(&sv1);

  sv_chop_left(&sv1, 2);
  print_sv(&sv1);

  sv_chop_right(&sv1, 2);
  print_sv(&sv1);

  return 0;
}
