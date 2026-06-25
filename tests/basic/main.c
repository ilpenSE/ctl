#include <stdio.h>
#define BASIC_IMPLEMENTATION
#include <basic.h>

int main() {
  char* temp = temp_sprintf("sa %d", 1);
  printf("temp = %s\n", temp);
  printf("temp_len = %zu\n", temp_len());
  printf("--------------------\n");
  char* temp2 = temp_sprintf("as %s", "kardes");
  printf("temp = %s\n", temp2);
  printf("temp_len = %zu\n", temp_len());
  printf("--------------------\n");
  int a = 0;
  scanf("%d", &a);
  printf("a (orig): %d\n", a);
  a = clamp(a, 0, 20);
  // 0 <= a <= 20
  printf("a = %d\n", a);
  return 0;
}
