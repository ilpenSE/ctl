#include <stdio.h>
#include <json.h>
#include <either.h>

int main() {
  Result(JsonState) res = json_read("test.json");
  if (RES_ISERR(&res)) {
    printf("ERROR: %s\n", RES_GETE(&res).message);
    return 1;
  }
  JsonState state = RES_UNWRAP(&res);

  json_deserialize(&state);

  return 0;
}
