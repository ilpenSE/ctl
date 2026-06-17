#ifndef JSON_H
#define JSON_H

#ifdef _MSC_VER
  #error "This header does not support MSVC, please don't use garbage slop compilers."
#endif

#ifdef __cplusplus
  #define JSONDEF extern "C"
#else
  #define JSONDEF extern
#endif

#include <stdbool.h>
#include <stddef.h>
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
  X(COLON)

typedef enum {
  JTK_EOF,
  #define X(eenum) JTK_##eenum,
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

typedef struct {
  JsonTokenType type;
  StringView lexeme;
} JsonToken;

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

typedef Array(JsonPair) JsonObject;
typedef Array(JsonValue) JsonArray;

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
  StringView file_path;
  size_t row; /* keep track of line count */
  size_t col; /* keep track of column number */
} JsonFile;

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

DECL_RESULT(Json, Json);
DECL_RESULT(JsonValue, JsonValue);

#ifndef __Result_String_defined
#define __Result_String_defined
DECL_RESULT(String, String);
#endif /* __Result_String_defined */

JSONDEF Result(Json) json_read(const char* file_path);
JSONDEF void json_free(Json* json);
JSONDEF void json_value_free(JsonValue* value);
JSONDEF bool json_parse(Json* json);
JSONDEF const char* json_tokentype_to_cstr(JsonTokenType type);

#ifdef JSON_IMPLEMENTATION
#include <ctype.h>
#include "futil.h"
// Static (internal) function declerations
static size_t _json_clamp(size_t a, size_t lower, size_t upper);

// Lexer functions
static JsonToken jlx_next(Json* json);
static void jlx_skip_whitespace(Json* json);
static JsonToken jlx_read_str(Json* json);
static JsonToken jlx_read_num(Json* json);
static JsonToken jlx_read_key(Json* json);
static char jlx_peek(Json* json);
static char jlx_advance(Json* json);

// Parser functions
static JsonArray jps_parse_array(Json* json);
static JsonObject jps_parse_object(Json* json);
static JsonValue jps_parse_value(Json* json);
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

#define _json_report_error(json, fmt, ...) \
  do { \
    JsonFile f = (json)->file; \
    fprintf(stderr, SV_FMT":%zu:%zu: ERROR: "fmt"\n", \
      SV_ARG(f.file_path), f.row + 1, f.col, ##__VA_ARGS__); \
  } while (0)

Result(Json) json_read(const char* file_path) {
  if (!file_path) return RES_ERR(Json, ERR_INVALID_ARG);
  Result(String) readen_res = read_entire_file(file_path);
  if (RES_ISERR(readen_res)) return RES_ERR(Json, readen_res.as.right.code);
  String content = readen_res.as.left;

  JsonFile file = {.file_path = sv(file_path)};
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
  JsonValue val = jps_parse_value(json);
  if (json->ps_curr.type != JTK_EOF) return false;
  json->value = val;
  return true;
}

void json_dump(Json* json) {
  while (1) {
    JsonToken tok = jlx_next(json);
    if (tok.type == JTK_EOF) break;
    printf("JsonToken{type=%s, lexeme=\""SV_FMT"\"}\n", json_tokentype_to_cstr(tok.type), SV_ARG(tok.lexeme));
  }
  json->lx_curr = 0;
}

// Lexer
static JsonToken jlx_next(Json* json) {
  jlx_skip_whitespace(json);
  char c = jlx_advance(json);

  switch (c) {
    case '{': return (JsonToken){.type = JTK_OCURLY, .lexeme=sv("{")};
    case '}': return (JsonToken){.type = JTK_CCURLY, .lexeme=sv("}")};
    case '[': return (JsonToken){.type = JTK_OBRACKET, .lexeme=sv("[")};
    case ']': return (JsonToken){.type = JTK_CBRACKET, .lexeme=sv("]")};
    case ':': return (JsonToken){.type = JTK_COLON, .lexeme=sv(":")};
    case ',': return (JsonToken){.type = JTK_COMMA, .lexeme=sv(",")};
    case '"': return jlx_read_str(json);
    case '\0': return (JsonToken){.lexeme=sv("<eof>")};
  }

  bool is_num_related = c == '-' || c == '.';

  if (isalpha((uchar_t)c) && !is_num_related) {
    return jlx_read_key(json);
  }

  if (isdigit((uchar_t)c) || is_num_related) {
    return jlx_read_num(json);
  }

  _json_report_error(json, "Unexpected token: '%c'", c);
  return (JsonToken){0};
}

static JsonToken jlx_read_str(Json* json) {
  size_t saved_curr = json->lx_curr;
  for (char c = jlx_advance(json); c != '"'; c = jlx_advance(json)) {
    if (c == '\\') {
      c = jlx_advance(json);
    }
  }
  StringView view = svs(&json->content, .start = saved_curr, .end = json->lx_curr - 1);
  return (JsonToken){.type=JTK_STRING, .lexeme = view};
}

static JsonToken jlx_read_num(Json* json) {
  size_t saved_curr = json->lx_curr - 1;
  char c = jlx_advance(json);
  while (isdigit((uchar_t)c) || c == 'e' || c == 'E' || c == '-' || c == '.') {
    c = jlx_advance(json);
  }
  json->lx_curr -= 1;

  StringView view = svs(&json->content, .start = saved_curr, .end = json->lx_curr);
  return (JsonToken){.type=JTK_NUMBER, .lexeme=view};
}

static JsonToken jlx_read_key(Json* json) {
  size_t saved_curr = json->lx_curr - 1;
  for (char c = jlx_advance(json); isalpha((uchar_t)c); c = jlx_advance(json));
  json->lx_curr -= 1;

  StringView view = svs(&json->content, .start = saved_curr, .end = json->lx_curr);
  if (sv_equals_cstr(&view, "true")) return (JsonToken){.type=JTK_TRUE};
  if (sv_equals_cstr(&view, "false")) return (JsonToken){.type=JTK_FALSE};
  if (sv_equals_cstr(&view, "null")) return (JsonToken){.type=JTK_NULL};

  _json_report_error(json, "Unexpected keyword: '"SV_FMT"'\nAvailable keywords are: `true`, `false` or `null`",
                     SV_ARG(view));
  return (JsonToken){0};
}

// Lexer helper functions
static char jlx_peek(Json* json) {
  String str = json->content;
  if (json->lx_curr >= str.len) return '\0';
  return str.data[_json_clamp(str.len, 0, json->lx_curr)];
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
static JsonArray jps_parse_array(Json* json) {
  JsonArray arr = {.h.memory=JSON_ARR_MEMORY_MANAGER};
  assert(json->ps_curr.type == JTK_OBRACKET &&
        "Cursor of parser doesn't look at open bracket ('['), maybe called from outside?");
  jps_advance(json);

  for (size_t idx = 1; true; idx++) {
    // Parse value
    JsonValue value = jps_parse_value(json);
    if (value.type == JSON_NOVALUE) {
      _json_report_error(json, "Error while parsing %zu%s value!", idx, (idx == 1 ? "st" : idx == 2 ? "nd" : idx == 3 ? "rd" : "th"));
      goto fail;
    }
    arr_push(&arr, value);

    // Handle ending or commas
    if (!jps_expect(json, JTK_COMMA)) {
      if (jps_expect(json, JTK_CBRACKET)) break;
      _json_report_error(json, "Exptected ',' or ']' but got '"SV_FMT"'", SV_ARG(json->ps_curr.lexeme));
      goto fail;
    }
  }

  return arr;
fail:
  return (JsonArray){0};
}

static JsonObject jps_parse_object(Json* json) {
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
      _json_report_error(json, "Expected string but got '"SV_FMT"'", SV_ARG(key_sv));
      goto fail;
    }
    if (!jps_interpret_str(json, key_sv, &str)) goto fail;
    pair.key = str;
    jps_advance(json);

    // Expect :
    if (!jps_expect(json, JTK_COLON)) {
      _json_report_error(json, "Expected ':' but got '"SV_FMT"'", SV_ARG(json->ps_curr.lexeme));
      goto fail;
    }

    // Parse value
    JsonValue* value = JSON_MALLOC(sizeof(JsonValue));
    *value = jps_parse_value(json);
    if (value->type == JSON_NOVALUE) {
      _json_report_error(json, "Error while parsing value of `"SV_FMT"`", SV_ARG(key_sv));
      goto fail;
    }
    pair.value = value;
    arr_push(&obj, pair);

    // Handle ending or commas
    if (!jps_expect(json, JTK_COMMA)) {
      if (jps_expect(json, JTK_CCURLY)) break;
      _json_report_error(json, "Exptected ',' or '}' but got '"SV_FMT"'", SV_ARG(json->ps_curr.lexeme));
      goto fail;
    }
  }

  return obj;
fail:
  return (JsonObject){0};
}

JsonValue jps_parse_value(Json* json) {
  JsonTokenType type = json->ps_curr.type;
  JsonValue val = {0};

  switch (type) {
  case JTK_STRING: {
    String str = {0};
    if (!jps_interpret_str(json, json->ps_curr.lexeme, &str)) goto fail;
    val.type = JSON_STRING;
    val.as.string = str;
  } break;
  case JTK_NUMBER: {
    // TODO: Interpret the number
    val.type = JSON_NUMBER;
    String str = str_from_sv(&json->ps_curr.lexeme, JSON_STR_MEMORY_MANAGER_FLAT);
    val.as.number = strtod(str.data, NULL);
  } break;
  case JTK_TRUE: {val.type = JSON_TRUE; val.as.boolean = true;} break;
  case JTK_FALSE: {val.type = JSON_FALSE; val.as.boolean = false;} break;
  case JTK_NULL: {val.type = JSON_NULL;} break;
  case JTK_OCURLY: {val.type = JSON_OBJECT; val.as.object = jps_parse_object(json);} break;
  case JTK_OBRACKET: {val.type = JSON_ARRAY; val.as.array = jps_parse_array(json);} break;
  default: _json_report_error(json, "Unexpected end of file"); goto fail;
  }

  if (type != JTK_OCURLY && type != JTK_OBRACKET)
    jps_advance(json);
  return val;
fail:
  return (JsonValue){0};
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
  *str = (String){.memory=JSON_STR_MEMORY_MANAGER};
  for (size_t i = 0; i < orig_sv.len; i++) {
    char ch = orig_sv.data[i];
    if (ch != ' ' && isspace((uchar_t)ch)) {
      _json_report_error(json, "Invalid space character in a string: '%c'", ch);
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
      default: _json_report_error(json, "Invalid escape character: '\\%c'", ch); return false;
      }
    } else {
      str_append(str, ch);
    }
  }
  return true;
}

static inline size_t _json_clamp(size_t a, size_t lower, size_t upper) {
  return a < lower ? lower : (a > upper ? upper : a);
}

#endif // JSON_IMPLEMENTATION

#endif // JSON_H
