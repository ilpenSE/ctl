#define SV_IMPLEMENTATION
#define STR_IMPLEMENTATION
#include "sv.h"
#include "str.h"

int main() {
  StringView sv = {0};
  bool is_empty_sv = sv_isempty(&sv);
  String s = str_from_sv(&sv);
  StringView sv2 = sv_from_str(&s, .end = s.len);
  return 0;
}
