#include <stdio.h>
#include "prettyp.h"

int main() {
  String a = str_new("Selamaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  String b = str_new("Selamaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  if (str_equals(&a, &b)) {
    pprint("equals");
  } else {
    pprint("NOT equals");
  }
  return 0;
}
