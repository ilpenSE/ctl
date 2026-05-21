#include <stdio.h>
#include "prettyp.h"

int main() {
  // TRIM
  String s1 = str_new("\t\n  hello!");
  String s2 = str_new(" hello! \r\t\n");
  String s3 = str_new("hello! \n");
  String s4 = str_new("");

  str_trim_left(&s1);
  str_trim_right(&s2);
  str_trim(&s3);
  str_trim(&s4);

  pprint("\"%s\"", s1.data);
  pprint("\"%s\"", s2.data);
  pprint("\"%s\"", s3.data);
  pprint("\"%s\"", s4.data);

  printf("------------------------------\n");

  // TRIM FOR STRING VIEWS
  StringView sv1 = sv_from_cstr("\n\r\t  hello! ");
  StringView sv2 = sv_from_cstr(" hello!\t\r\n  ");
  StringView sv3 = sv_from_cstr(" \t  hello!  \n");

  StringView tsv1 = sv_trim_left(sv1);
  StringView tsv2 = sv_trim_right(sv2);
  StringView tsv3 = sv_trim(sv3);

  print_sv(&tsv1);
  print_sv(&tsv2);
  print_sv(&tsv3);
  return 0;
}
