#include <stdio.h>
#include "prettyp.h"

int main() {
  String s = str_new("selam");
  print_str(&s);

  if (!str_shrink_to_fit(&s)) printf("oh\n");
  else print_str(&s);
  
  str_clear(&s);
  print_str(&s);
  
  str_free(&s);
  print_str(&s);
  return 0;
}
