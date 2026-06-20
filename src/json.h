/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef JSON_H
#define JSON_H

#ifdef __cplusplus
  #define JSONDEF extern "C"
#else
  #define JSONDEF extern
#endif

#include <stdbool.h>
#include <stddef.h>
#include "basic.h"
#include "either.h"
#include "array.h"
#include "sv.h"

#define JSON_TOKEN_TYPES \
  X(OCURLY) /* { */ \
  X(CCURLY) /* } */ \
  X(OBRACKET) /* [ */ \
  X(CBRACKET) /* ] */ \
  X(STRING) \
  X(NUMBER) \
  X(TRUE) \
  X(FALSE) \
  X(NULL) \
  X(COMMA) \
  X(COLON) \
  X(UNKNOWN)

typedef enum {
  JTK_EOF,
  #define X(eenum) \
    JTK_##eenum,
  JSON_TOKEN_TYPES
  #undef X
  _JsonTokenType_count,
} JsonTokenType;

typedef enum {
  JSON_NOVALUE = 0,
  JSON_OBJECT,
  JSON_ARRAY,
  JSON_STRING,
  JSON_NUMBER,
  JSON_TRUE,
  JSON_FALSE,
  JSON_NULL,
  _JsonValueType_count,
} JsonValueType;

typedef struct JsonValue JsonValue;
typedef struct {
  String key;
  JsonValue* value;
} JsonPair;

#ifndef __Array_JsonPair_defined
#define __Array_JsonPair_defined
DECL_ARRAY(JsonPair, JsonPair);
#endif /* __Array_JsonPair_defined */

#ifndef __Array_JsonValue_defined
#define __Array_JsonValue_defined
DECL_ARRAY(JsonValue, JsonValue);
#endif /* __Array_JsonValue_defined */

#ifndef __JsonObject_defined
#define __JsonObject_defined
typedef Array(JsonPair) JsonObject;
#endif /* __JsonObject_defined */

#ifndef __JsonArray_defined
#define __JsonArray_defined
typedef Array(JsonValue) JsonArray;
#endif /* __JsonArray_defined */

struct JsonValue {
  JsonValueType type;
  union {
    String string;
    JsonObject object;
    JsonArray array;
    double number;
    bool boolean;
  } as;
};

typedef struct {
  StringView path;
  size_t row; /* keep track of line count */
  size_t col; /* keep track of column number */
} JsonFile;

typedef struct {
  JsonTokenType type;
  StringView lexeme;
  char ch; /* Used when type is JTK_UNKNOWN */
  JsonFile file; /* Also track the file and we can get better error reporting in parser */
} JsonToken;

/*
  Context structure of JSON Deserializer
*/
typedef struct {
  String content;
  JsonFile file;
  JsonValue value;
  size_t lx_curr; /* Cursor of lexer */
  JsonToken ps_curr; /* Cursor of parser */
} Json;

/* Declaring the Results */
#ifndef __Result_Json_defined
#define __Result_Json_defined
DECL_RESULT(Json, Json);
#endif /* __Result_Json_defined */

#ifndef __Result_JsonValue_defined
#define __Result_JsonValue_defined
DECL_RESULT(JsonValue, JsonValue);
#endif /* __Result_JsonValue_defined */

#ifndef __Result_JsonObject_defined
#define __Result_JsonObject_defined
DECL_RESULT(JsonObject, JsonObject);
#endif /* __Result_JsonObject_defined */

#ifndef __Result_JsonArray_defined
#define __Result_JsonArray_defined
DECL_RESULT(JsonArray, JsonArray);
#endif /* __Result_JsonArray_defined */

JSONDEF Result(Json) json_read(const char* file_path);
JSONDEF void json_free(Json* json);
JSONDEF void json_value_free(JsonValue* value);
JSONDEF bool json_parse(Json* json);
JSONDEF const char* json_tokentype_to_cstr(JsonTokenType type);

/*
  Check if given JsonValue is a specific type
  Simply checks JsonValue.type field.
*/
JSONDEF bool json_is_string(const JsonValue* json_value);
JSONDEF bool json_is_number(const JsonValue* json_value);
JSONDEF bool json_is_boolean(const JsonValue* json_value);
JSONDEF bool json_is_null(const JsonValue* json_value);
JSONDEF bool json_is_object(const JsonValue* json_value);
JSONDEF bool json_is_array(const JsonValue* json_value);

/*
  Extracts specific type from JsonValue
  It checks the type (calls json_is* functions) that's why it returns Result(T)
  There's no "as_null" because null means there's no value preset.
  After checking it return JsonValue.as.T field. (T can be string, number etc.)
*/
JSONDEF Result(String) json_as_string(const JsonValue* json_value);
JSONDEF Result(double) json_as_number(const JsonValue* json_value);
JSONDEF Result(bool) json_as_boolean(const JsonValue* json_value);
JSONDEF Result(JsonObject) json_as_object(const JsonValue* json_value);
JSONDEF Result(JsonArray) json_as_array(const JsonValue* json_value);

/*
  Gets specific type without checking. (Returns JsonValue.as.T)
  That's why it's called "unwrap" (It can lead to UB if there's no check)
*/
JSONDEF String json_unwrap_string(const JsonValue* json_value);
JSONDEF double json_unwrap_number(const JsonValue* json_value);
JSONDEF bool json_unwrap_boolean(const JsonValue* json_value);
JSONDEF JsonObject json_unwrap_object(const JsonValue* json_value);
JSONDEF JsonArray json_unwrap_array(const JsonValue* json_value);

// IMPLEMENTATION BEGIN
#ifdef JSON_IMPLEMENTATION
#include <ctype.h>
#include "futil.h"

// Lexer functions
static JsonToken jlx_next(Json* json);
static void jlx_skip_whitespace(Json* json);
static JsonToken jlx_read_str(Json* json);
static JsonToken jlx_read_num(Json* json);
static JsonToken jlx_read_key(Json* json);
static char jlx_peek(Json* json);
static char jlx_advance(Json* json);

// Parser functions
static Result(JsonArray) jps_parse_array(Json* json);
static Result(JsonObject) jps_parse_object(Json* json);
static Result(JsonValue) jps_parse_value(Json* json);
static bool jps_expect(Json* json, JsonTokenType expected_type);
static JsonToken jps_advance(Json* json);
static bool jps_interpret_str(Json* json, StringView orig_sv, String* str);

#define JSON_MALLOC malloc
#define JSON_REALLOC realloc
#define JSON_FREE free

#define JSON_STR_MEMORY_MANAGER \
  (StringMemory){.allocator=JSON_MALLOC, .freer=JSON_FREE, .reallocator=JSON_REALLOC}

#define JSON_STR_MEMORY_MANAGER_FLAT \
  .allocator=JSON_MALLOC, .freer=JSON_FREE, .reallocator=JSON_REALLOC

#define JSON_ARR_MEMORY_MANAGER \
  (ArrayMemory){.allocator=JSON_MALLOC, .freer=JSON_FREE, .reallocator=JSON_REALLOC}

#define jlx_report_error(json, fmt, ...) \
  do { \
    fprintf(stderr, SV_FMT":%zu:%zu: ERROR: "fmt"\n", \
      SV_ARG((json)->file.path), (json)->file.row + 1, (json)->file.col, ##__VA_ARGS__); \
  } while (0)

#define jps_report_error(token, fmt, ...) \
  do { \
    fprintf(stderr, SV_FMT":%zu:%zu: ERROR: "fmt"\n", \
      SV_ARG((token).file.path), (token).file.row + 1, (token).file.col, ##__VA_ARGS__); \
  } while (0)

#define jps_report_error_exp(token, expected) \
  do { \
    if ((token).ch != '\0') { \
      jps_report_error((token), "Expected %s but got '%c'", \
        (expected), (token).ch); \
    } else { \
      jps_report_error((token), "Expected %s but got '"SV_FMT"'", \
        (expected), SV_ARG((token).lexeme)); \
    } \
  } while (0)

Result(Json) json_read(const char* file_path) {
  if (!file_path) return RES_ERR(Json, ERR_INVALID_ARG);
  Result(String) readen_res = read_entire_file(file_path);
  if (RES_ISERR(readen_res)) return RES_ERR(Json, readen_res.as.right.code);
  String content = readen_res.as.left;

  JsonFile file = {.path = sv(file_path)};
  Json json = {
    .content = content,
    .file = file,
  };
  return RES_OK(Json, json);
}

void json_value_free(JsonValue* value) {
  switch (value->type) {
  case JSON_OBJECT: {
    JsonObject obj = value->as.object;
    arr_foreach(&obj, it) {
      // typeof(*it) == JsonPair;
      json_value_free(it->value);
      JSON_FREE(it->value);
    }
    arr_free(&obj);
  } break;
  case JSON_ARRAY: {
    JsonArray arr = value->as.array;
    arr_foreach(&arr, it) {
      json_value_free(it);
    }
    arr_free(&arr);
  } break;
  case JSON_STRING: {
    str_free(&value->as.string);
  }; break;
  default: break;
  }
}

void json_free(Json* json) {
  if (!json) return;
  json_value_free(&json->value);
  str_free(&json->content);
  *json = (Json){0};
}

const char* json_tokentype_to_cstr(JsonTokenType type) {
  switch (type) {
  case JTK_EOF: return "JTK_EOF";
  #define X(eenum) \
    case JTK_##eenum: return "JTK_" STRINGIFY(eenum);
  JSON_TOKEN_TYPES
  #undef X
  default: return NULL;
  }
  UNREACHABLE("json_tokentype_to_cstr");
}

bool json_parse(Json* json) {
  if (!json) return false;
  json->ps_curr = jlx_next(json);
  Result(JsonValue) val_res = jps_parse_value(json);
  if (RES_ISERR(val_res)) return false;
  if (json->ps_curr.type != JTK_EOF) return false;
  json->value = RES_UNWRAP(val_res);
  return true;
}

void json_dump(Json* json) {
  while (1) {
    JsonToken tok = jlx_next(json);
    if (tok.type == JTK_EOF) break;
    printf("JsonToken{type=%s, lexeme=\""SV_FMT"\", ROW=%zu, COL=%zu}\n",
      json_tokentype_to_cstr(tok.type), SV_ARG(tok.lexeme), tok.file.row, tok.file.col);
  }
  json->lx_curr = 0;
}

// Lexer
static JsonToken jlx_next(Json* json) {
  jlx_skip_whitespace(json);
  char c = jlx_advance(json);

  switch (c) {
    case '{': return (JsonToken){.type = JTK_OCURLY, .lexeme=sv("{"), .file=json->file};
    case '}': return (JsonToken){.type = JTK_CCURLY, .lexeme=sv("}"), .file=json->file};
    case '[': return (JsonToken){.type = JTK_OBRACKET, .lexeme=sv("["), .file=json->file};
    case ']': return (JsonToken){.type = JTK_CBRACKET, .lexeme=sv("]"), .file=json->file};
    case ':': return (JsonToken){.type = JTK_COLON, .lexeme=sv(":"), .file=json->file};
    case ',': return (JsonToken){.type = JTK_COMMA, .lexeme=sv(","), .file=json->file};
    case '"': return jlx_read_str(json);
    case '\0': return (JsonToken){.lexeme=sv("<eof>"), .file=json->file};
  }

  bool is_num_related = c == '-' || c == '.';

  if (isalpha((uchar_t)c) && !is_num_related) {
    return jlx_read_key(json);
  }

  if (isdigit((uchar_t)c) || is_num_related) {
    return jlx_read_num(json);
  }

  jlx_report_error(json, "Unexpected token: '%c'", c);
  return (JsonToken){.type=JTK_UNKNOWN, .ch=c, .file=json->file};
}

static JsonToken jlx_read_str(Json* json) {
  char c = jlx_peek(json);
  size_t i = 0;
  while (c != '"') {
    bool is_escape = c == '\\';
    if (is_escape) jlx_advance(json);
    jlx_advance(json);
    c = jlx_peek(json);
    if (is_escape) i += 2;
    else i += 1;
  }
  StringView view = svs(&json->content, .start=json->lx_curr - i, .end=json->lx_curr);
  jlx_advance(json);
  return (JsonToken){.type=JTK_STRING, .lexeme=view, .file=json->file};
}

static JsonToken jlx_read_num(Json* json) {
  char c = jlx_peek(json);
  size_t i = 1;
  while (isdigit((uchar_t)c) || c == 'e' || c == 'E' || c == '.' || c == '-') {
    jlx_advance(json);
    c = jlx_peek(json);
    i++;
  }

  StringView view = svs(&json->content, .start=json->lx_curr - i, .end=json->lx_curr);
  return (JsonToken){.type=JTK_NUMBER, .lexeme=view, .file=json->file};
}

static JsonToken jlx_read_key(Json* json) {
  char c = jlx_peek(json);
  size_t i = 1;
  while (isalnum((uchar_t)c)) {
    jlx_advance(json);
    c = jlx_peek(json);
    i++;
  }

  StringView view = svs(&json->content, .start=json->lx_curr - i, .end=json->lx_curr);
  if (sv_equals_cstr(&view, "true")) return (JsonToken){.type=JTK_TRUE, .file=json->file};
  if (sv_equals_cstr(&view, "false")) return (JsonToken){.type=JTK_FALSE, .file=json->file};
  if (sv_equals_cstr(&view, "null")) return (JsonToken){.type=JTK_NULL, .file=json->file};

  return (JsonToken){.type=JTK_UNKNOWN, .lexeme=view, .file=json->file};
}

// Lexer helper functions
static char jlx_peek(Json* json) {
  String str = json->content;
  if (json->lx_curr >= str.len) return '\0';
  return str.data[clamp(str.len, 0, json->lx_curr)];
}

static char jlx_advance(Json* json) {
  char c = jlx_peek(json);
  if (c != '\0') json->lx_curr += 1;
  if (c == '\n') {
    json->file.row += 1;
    json->file.col = 0;
  } else json->file.col += 1;
  return c;
}

static void jlx_skip_whitespace(Json* json) {
  for (;isspace((uchar_t)jlx_peek(json)); jlx_advance(json))
    ;;
}

// Parser
static Result(JsonArray) jps_parse_array(Json* json) {
  JsonArray arr = {.h.memory=JSON_ARR_MEMORY_MANAGER};
  assert(json->ps_curr.type == JTK_OBRACKET &&
        "Cursor of parser doesn't look at open bracket ('['), maybe called from outside?");
  jps_advance(json);

  for (size_t idx = 1; true; idx++) {
    // Parse value
    Result(JsonValue) value_res = jps_parse_value(json);
    if (RES_ISERR(value_res)) goto fail;
    JsonValue value = RES_UNWRAP(value_res);
    if (value.type == JSON_NOVALUE) {
      jps_report_error(json->ps_curr, "Error while parsing %zu%s value!",
        idx, (idx == 1 ? "st" : idx == 2 ? "nd" : idx == 3 ? "rd" : "th"));
      goto fail;
    }
    arr_push(&arr, value);

    // Handle ending or commas
    if (!jps_expect(json, JTK_COMMA)) {
      if (jps_expect(json, JTK_CBRACKET)) break;
      jps_report_error_exp(json->ps_curr, "',' or ']'");
      goto fail;
    }
  }

  return RES_OK(JsonArray, arr);
fail:
  return RES_ERR(JsonArray, ERR_INTERNAL);
}

static Result(JsonObject) jps_parse_object(Json* json) {
  JsonObject obj = {.h.memory=JSON_ARR_MEMORY_MANAGER};
  assert(json->ps_curr.type == JTK_OCURLY &&
         "Cursor of parser doesn't look at open curly brace ('{'), maybe called from outside?");
  jps_advance(json);

  while (1) {
    // STRING, COLON, VALUE
    StringView key_sv = json->ps_curr.lexeme;
    String str = {.memory=JSON_STR_MEMORY_MANAGER};
    JsonPair pair = {0};
    if (json->ps_curr.type != JTK_STRING) {
      jps_report_error_exp(json->ps_curr, "string");
      goto fail;
    }
    if (!jps_interpret_str(json, key_sv, &str)) goto fail;
    pair.key = str;
    jps_advance(json);

    // Expect :
    if (!jps_expect(json, JTK_COLON)) {
      jps_report_error_exp(json->ps_curr, "':'");
      goto fail;
    }

    // Parse value
    Result(JsonValue) value_res = jps_parse_value(json);
    if (RES_ISERR(value_res)) goto fail;
    JsonValue* value = JSON_MALLOC(sizeof(JsonValue));
    *value = RES_UNWRAP(value_res);
    if (value->type == JSON_NOVALUE) {
      jps_report_error(json->ps_curr, "Error while parsing value of `"SV_FMT"`", SV_ARG(key_sv));
      goto fail;
    }
    pair.value = value;
    arr_push(&obj, pair);

    // Handle ending or commas
    if (!jps_expect(json, JTK_COMMA)) {
      if (jps_expect(json, JTK_CCURLY)) break;
      jps_report_error_exp(json->ps_curr, "',' or '}'");
      goto fail;
    }
  }

  return RES_OK(JsonObject, obj);
fail:
  return RES_ERR(JsonObject, ERR_INTERNAL);
}

Result(JsonValue) jps_parse_value(Json* json) {
  JsonToken tok = json->ps_curr;
  JsonValue val = {0};

  switch (tok.type) {
  case JTK_STRING: {
    String str = {0};
    if (!jps_interpret_str(json, tok.lexeme, &str)) goto fail;
    val.type = JSON_STRING;
    val.as.string = str;
  } break;
  case JTK_NUMBER: {
    // TODO: Interpret the number
    val.type = JSON_NUMBER;
    String str = str_from_sv(&tok.lexeme, JSON_STR_MEMORY_MANAGER_FLAT);
    val.as.number = strtod(str.data, NULL);
  } break;
  case JTK_TRUE: {val.type = JSON_TRUE; val.as.boolean = true;} break;
  case JTK_FALSE: {val.type = JSON_FALSE; val.as.boolean = false;} break;
  case JTK_NULL: {val.type = JSON_NULL;} break;
  case JTK_OCURLY: {
    Result(JsonObject) obj_res = jps_parse_object(json);
    if (RES_ISERR(obj_res)) goto fail;
    val.type = JSON_OBJECT;
    val.as.object = RES_UNWRAP(obj_res);
  } break;
  case JTK_OBRACKET: {
    Result(JsonArray) arr_res = jps_parse_array(json);
    if (RES_ISERR(arr_res)) goto fail;
    val.type = JSON_ARRAY;
    val.as.array = RES_UNWRAP(arr_res);
  } break;
  case JTK_UNKNOWN: {
    val.type = JSON_NOVALUE;
    jps_report_error_exp(tok, "'true', 'false' or 'null'");
  } break;
  default: jps_report_error(tok, "Unexpected end of file"); goto fail;
  }

  if (tok.type != JTK_OCURLY && tok.type != JTK_OBRACKET)
    jps_advance(json);
  return RES_OK(JsonValue, val);
fail:
  return RES_ERR(JsonValue, ERR_INTERNAL);
}

// Parser helper functions
static bool jps_expect(Json* json, JsonTokenType expected_type) {
  if (json->ps_curr.type != expected_type) return false;
  jps_advance(json);
  return true;
}

static JsonToken jps_advance(Json* json) {
  JsonToken tok = jlx_next(json);
  json->ps_curr = tok;
  return tok;
}

static bool jps_interpret_str(Json* json, StringView orig_sv, String* str) {
  JsonToken tok = json->ps_curr;
  *str = (String){.memory=JSON_STR_MEMORY_MANAGER};
  for (size_t i = 0; i < orig_sv.len; i++) {
    char ch = orig_sv.data[i];
    if (ch != ' ' && isspace((uchar_t)ch)) {
      jps_report_error(tok, "Invalid space character in a string: '%c'", ch);
      return false;
    }
    if (ch == '\\') {
      i++;
      ch = orig_sv.data[i];
      switch (ch) {
      case 'n': str_append(str, '\n'); break;
      case 'f': str_append(str, '\f'); break;
      case 't': str_append(str, '\t'); break;
      case 'r': str_append(str, '\r'); break;
      case 'b': str_append(str, '\b'); break;
      case '\\': str_append(str, '\\'); break;
      default: jps_report_error(tok, "Invalid escape character: '\\%c'", ch); return false;
      }
    } else {
      str_append(str, ch);
    }
  }
  return true;
}

// String
bool json_is_string(const JsonValue* json_value) {
  return json_value->type == JSON_STRING;
}
Result(String) json_as_string(const JsonValue* json_value) {
  if (!json_is_string(json_value)) return RES_ERR(String, ERR_INVALID_ARG);
  return RES_OK(String, json_value->as.string);
}
String json_unwrap_string(const JsonValue* json_value) {
  return json_value->as.string;
}

// Number
bool json_is_number(const JsonValue* json_value) {
  return json_value->type == JSON_NUMBER;
}
Result(double) json_as_number(const JsonValue* json_value) {
  if (!json_is_number(json_value)) return RES_ERR(double, ERR_INVALID_ARG);
  return RES_OK(double, json_value->as.number);
}
double json_unwrap_number(const JsonValue* json_value) {
  return json_value->as.number;
}

// Boolean
bool json_is_boolean(const JsonValue* json_value) {
  return json_value->type == JSON_TRUE || json_value->type == JSON_FALSE;
}
Result(bool) json_as_boolean(const JsonValue* json_value) {
  if (!json_is_boolean(json_value)) return RES_ERR(bool, ERR_INVALID_ARG);
  return RES_OK(bool, json_value->as.boolean);
}
bool json_unwrap_boolean(const JsonValue* json_value) {
  return json_value->as.boolean;
}

// Object
bool json_is_object(const JsonValue* json_value) {
  return json_value->type == JSON_TRUE || json_value->type == JSON_FALSE;
}
Result(JsonObject) json_as_object(const JsonValue* json_value) {
  if (!json_is_object(json_value)) return RES_ERR(JsonObject, ERR_INVALID_ARG);
  return RES_OK(JsonObject, json_value->as.object);
}
JsonObject json_unwrap_object(const JsonValue* json_value) {
  return json_value->as.object;
}

// Array
bool json_is_array(const JsonValue* json_value) {
  return json_value->type == JSON_TRUE || json_value->type == JSON_FALSE;
}
Result(JsonArray) json_as_array(const JsonValue* json_value) {
  if (!json_is_array(json_value)) return RES_ERR(JsonArray, ERR_INVALID_ARG);
  return RES_OK(JsonArray, json_value->as.array);
}
JsonArray json_unwrap_array(const JsonValue* json_value) {
  return json_value->as.array;
}

// Null (type)
bool json_is_null(const JsonValue* json_value) {
  return json_value->type == JSON_NULL;
}

#endif // JSON_IMPLEMENTATION
// IMPLEMENTATION END
#endif /* JSON_H */
