#include <stdio.h>
#include <json.h>
#include <either.h>

int main() {
  Result(Json) json_res = json_read("test.json");
  if (RES_ISERR(json_res)) {
    fprintf(stderr, "ERROR: Failed to read json file: %s", err_tostr(json_res.as.right.code));
    return 1;
  }
  Json json = RES_UNWRAP(json_res);

#if 0
  json_dump(&json);
#else
  if (!json_parse(&json)) return 1;
  JsonObject obj = json.value.as.object;
  arr_foreach(&obj, it) {
    JsonPair pair = *it;
    char buf[1024];
    switch(pair.value->type) {
    case JSON_NUMBER:
      snprintf(buf, sizeof(buf), "%f", pair.value->as.number);
      break;
    case JSON_TRUE:
    case JSON_FALSE:
      snprintf(buf, sizeof(buf), "%s", pair.value->as.boolean ? "true" : "false");
      break;
    case JSON_ARRAY:
      snprintf(buf, sizeof(buf), "<array>");
      break;
    case JSON_STRING:
      snprintf(buf, sizeof(buf), "%s", pair.value->as.string.data);
      break;
    default:
      snprintf(buf, sizeof(buf), "unknown");
      break;
    }
    printf("Key: "SV_FMT" => %s\n", SV_ARG(pair.key), buf);
  }
#endif
  return 0;
}
