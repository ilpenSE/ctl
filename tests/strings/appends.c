#include <stdio.h>
#include "prettyp.h"

int main() {
  String s = str_new("kanka");
  print_str(&s);

  str_append(&s, '!');
  str_append(&s, ' ');
  print_str(&s); // expected: "kanka! "

  str_cat(&s, "hello ");
  print_str(&s); // expected: "kanka! hello "

  String c = str_new("world");
  str_cat_str(&s, &c);
  print_str(&s); // expected: "kanka! hello world"
  print_str(&c); // expected: "world"

  str_cat(&s, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
  print_str(&s);

  String a = str_new("BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB");
  str_cat_str(&s, &a);
  print_str(&s);
  print_str(&a);

  String b = str_new("ab");
  str_repeat(&b, 15);
  print_str(&b);
  
  str_free(&s);
  str_free(&a);
  str_free(&b);
  str_free(&c);
  return 0;
}
