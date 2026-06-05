#ifndef JSON_H
#define JSON_H

// NOT COMPLETED

#ifdef __cplusplus
  #define JSONDEF extern "C"
#else
  #define JSONDEF extern
#endif

#include <stdbool.h>
#include <stddef.h>
#include <limits.h>

// JSON errors for user
#define JSON_ERROR(js, fmt, ...) do { \
  fprintf(stderr, "ERROR: " fmt " at %s:%zu\n", ##__VA_ARGS__, (js)->jsonPath, (js)->jsonLine); \
  exit(1); \
  } while (0)

#include "sv.h"
#include "str.h"
#include "either.h"

typedef struct JsonValue JsonValue;

typedef enum {
  TK_EOF,
  TK_OBJ_BEG, TK_OBJ_END, TK_ARR_BEG, TK_ARR_END,
  TK_COLON, TK_COMMA, TK_STRING, TK_NUMBER,
  TK_NULL, TK_TRUE, TK_FALSE
} TokenType;

typedef struct {
  StringView lexeme;
  TokenType type;
} Token;

typedef enum {
  JSON_NULL, JSON_BOOL, JSON_NUMBER,
  JSON_STRING, JSON_ARRAY, JSON_OBJECT
} JsonType;

typedef struct {
  StringView key;
  JsonValue* value;
} JsonMember;

DECL_VECTOR(JsonMember, JsonMember);
DECL_VECTOR(JsonValue, JsonValue);
typedef Vector(JsonMember) JsonObject;
typedef Vector(JsonValue) JsonArray;

struct JsonValue {
  JsonType type;
  union {
    bool boolean;
    double number;
    StringView str;
    JsonArray array;
    JsonObject object;
  } as;
};

// Temporary structs, DO NOT use!
typedef struct {
  String data;
  Token ps_curr;
  size_t lx_curr;
  size_t jsonLine;
  char jsonPath[PATH_MAX];
} JsonState;

DECL_RESULT(JsonValue, JsonValue);
DECL_RESULT(JsonState, JsonState);
DECL_OPTION(StringView, StringView);
DECL_OPTION(double, double);
DECL_OPTION(bool, bool);
DECL_OPTION(JsonArray, JsonArray);
DECL_OPTION(JsonObject, JsonObject);

#define MAX_TOKEN_LIMIT 1024

// Reads the whole JSON file and creates JsonState struct
JSONDEF Result(JsonState) json_read(const char* file_path);

// Parses the JsonState struct.
JSONDEF Result(JsonValue) json_deserialize(JsonState* js);

JSONDEF bool json_isstring(const JsonValue* jv);
JSONDEF bool json_isnumber(const JsonValue* jv);
JSONDEF bool json_isnull  (const JsonValue* jv);
JSONDEF bool json_isbool  (const JsonValue* jv);
JSONDEF bool json_isarray (const JsonValue* jv);
JSONDEF bool json_isobject(const JsonValue* jv);

JSONDEF Option(string)     json_asstring(const JsonValue* jv);
JSONDEF Option(double)     json_asnumber(const JsonValue* jv);
JSONDEF Option(bool)       json_asbool  (const JsonValue* jv);
JSONDEF Option(JsonArray)  json_asarray (const JsonValue* jv);
JSONDEF Option(JsonObject) json_asobject(const JsonValue* jv);

// Helpers
JSONDEF const char* tokentype_to_str(const TokenType t);

#ifdef JSON_IMPLEMENTATION

#include <ctype.h>

// Lexer
static void  lx_skipWhitespace(JsonState* js);
static char  lx_advance(JsonState* js);
static char  lx_peek(JsonState* js);
static Token lx_next(JsonState* js);
static Token lx_readString(JsonState* js);
static Token lx_readNumber(JsonState* js);
static Token lx_readKeywrd(JsonState* js);

// Parser
static Token ps_next(JsonState* js);
static Token ps_peek(JsonState* js);
static Token ps_advance(JsonState* js);
static JsonValue ps_parseValue(JsonState* js);
static JsonObject ps_parseObject(JsonState* js);
static JsonArray ps_parseArray(JsonState* js);

Result(JsonState) json_read(const char* file_path) {
  Result(String) str_res = read_entire_file(file_path);
  if (RES_ISERR(str_res)) return RES_ERR(JsonState, RES_GETE(str_res));
  String str = RES_UNWRAP(str_res);

// TODO
  // Declare string, dont use str_newn beacuse it copies
  string s = {.data=buf, .cap=read_bytes + 1, .len=read_bytes};
  JsonState state = {.data=s};

  memcpy(state.jsonPath, file_path, file_path_len);

  fclose(f);
  return RES_OK(JsonState, state);
}

Result(JsonValue) json_deserialize(JsonState* js) {
  if (!js) return RES_ERR(JsonValue, ERR_NULL_PTR, "");
  JsonValue jv = {0};
  js->ps_curr = lx_next(js);

  // PARSER NOT IMPLEMENTED YET
  // WE JUST DUMP ALL TOKENS FOR NOW
  Token t = ps_next(js);
  while (t.type != TK_EOF) {
    printf("Token{ TokenType=%s, lexeme=\"%s\" }\n", tokentype_to_str(t.type), t.lexeme.data);
    t = ps_next(js);
  }

  return RES_OK(JsonValue, jv);
}

// Lexer
static char lx_advance(JsonState* js) {
  if (!js) return 0;
  char old = str_idx(&js->data, js->lx_curr);
  js->lx_curr += (old != 0);
  return old;
}

static char lx_peek(JsonState* js) {
  if (!js) return 0;
  return str_idx(&js->data, js->lx_curr);
}

static void lx_skipWhitespace(JsonState* js) {
  while (isspace((uchar_t)lx_peek(js))) {
    js->jsonLine += 1;
    lx_advance(js);
  }
}

static Token lx_next(JsonState* js) {
  if (!js) return (Token) {0};
  lx_skipWhitespace(js);

  char c = lx_advance(js);
  switch(c) {
  case '{':  return (Token) {.type=TK_OBJ_BEG};
  case '}':  return (Token) {.type=TK_OBJ_END};
  case '[':  return (Token) {.type=TK_ARR_BEG};
  case ']':  return (Token) {.type=TK_ARR_END};
  case ':':  return (Token) {.type=TK_COLON};
  case ',':  return (Token) {.type=TK_COMMA};
  case '\0': return (Token) {.type=TK_EOF};
  case '"':  return lx_readString(js);
  }

  if (isalpha((uchar_t)c)) {
    js->lx_curr -= 1;
    return lx_readKeywrd(js);
  }

  if (isdigit((uchar_t)c) || c == '-') {
    js->lx_curr -= 1;
    return lx_readNumber(js);
  }

  return (Token) {0};
}

static Token lx_readString(JsonState* js) {
  string lex = str_new("");

  for (char c = lx_advance(js); c != '"'; c = lx_advance(js)) {
    if (c == '\\') {
      c = lx_advance(js);
      switch (c) {
      case 'n':  str_append(&lex, '\n'); break;
      case 't':  str_append(&lex, '\t'); break;
      case 'r':  str_append(&lex, '\r'); break;
      case 'b':  str_append(&lex, '\b'); break;
      case 'f':  str_append(&lex, '\f'); break;
      case '\\': str_append(&lex, '\\'); break;
      case '"':  str_append(&lex, '"');  break;
      default: {
        fprintf(stderr, "Unknown escape character: \\%c\n", c);
        return (Token){0};
      }
      }
    } else if (isspace((uchar_t)c)) {
      JSON_ERROR(js, "Illegal char in string \"%s\"", lex.data);
    } else if (c == '\0') {
      JSON_ERROR(js, "Unexpected EOF");
    } else {
      str_append(&lex, c);
    }
  }

  return (Token) {.type=TK_STRING, .lexeme=lex};
}

static Token lx_readNumber(JsonState* js) {
  if (!js) return (Token) {.type=TK_NUMBER};
  char c = lx_advance(js);
  string s = str_new("");
  
  // handle minus at the beginning
  if (c == '-') {
    str_append(&s, '-');
    c = lx_advance(js);
  }

  // handle leading zero situations
  if (c == '0') {
    c = lx_advance(js);
    if (isdigit((uchar_t)c)) JSON_ERROR(js, "Unexpected leading zero");
    else js->lx_curr -= 1;
  }

  while (1) {
    if (isdigit((uchar_t)c) || c == '.' || c == 'e' || c == 'E') {
      str_append(&s, c);
      c = lx_advance(js);
    } else if (c == '-' || c == '\0') JSON_ERROR(js, "Unexpected number");
    else break;
  }

  if (c != '\0') js->lx_curr -= 1;
  return (Token) {.type=TK_NUMBER, .lexeme=s};
}

static Token lx_readKeywrd(JsonState* js) {
  if (!js) return (Token) {0};
  char buf[6] = {0};

  char c = lx_advance(js);
  int i = 0;
  while (isalpha((uchar_t)c) && i < 5) {
    buf[i++] = c;
    c = lx_advance(js);
  }

  // rewind that non-alpha character
  if (c != '\0') js->lx_curr -= 1;

  if (strcmp(buf, "true") == 0) return (Token) {.type=TK_TRUE};
  if (strcmp(buf, "false") == 0) return (Token) {.type=TK_FALSE};
  if (strcmp(buf, "null") == 0) return (Token) {.type=TK_NULL};
  JSON_ERROR(js, "Unknown keyword: %s\n", buf);
  return (Token) {0};
}

// Parser
static Token ps_next(JsonState* js) {
  if (!js) return (Token) {0};
  Token old = js->ps_curr;
  js->ps_curr = lx_next(js);
  return old;
}

static Token ps_peek(JsonState* js) {
  if (!js) return (Token) {0};
  return js->ps_curr;
}

static JsonValue ps_parseValue(JsonState* js) {
  if (!js) return (JsonValue) {0};
  TODO("ps_parseValue");
}

static JsonObject ps_parseObject(JsonState* js) {
  if (!js) return (JsonObject) {0};
  TODO("ps_parseObject");
}

static JsonArray ps_parseArray(JsonState* js) {
  if (!js) return (JsonArray) {0};
  TODO("ps_parseArray");
}

const char* tokentype_to_str(const TokenType t) {
  switch (t) {
  case TK_OBJ_BEG: return "TK_OBJ_BEG";
  case TK_OBJ_END: return "TK_OBJ_END";
  case TK_ARR_BEG: return "TK_ARR_BEG";
  case TK_ARR_END: return "TK_ARR_END";
  case TK_COLON: return "TK_COLON";
  case TK_COMMA: return "TK_COMMA";
  case TK_STRING: return "TK_STRING";
  case TK_NUMBER: return "TK_NUMBER";
  case TK_NULL: return "TK_NULL";
  case TK_TRUE: return "TK_TRUE";
  case TK_FALSE: return "TK_FALSE";
  case TK_EOF: return "TK_EOF";
  }
  return "";
}

// is-functions
bool json_isstring(const JsonValue* jv){
  return jv->type == JSON_STRING;
}
bool json_isnumber(const JsonValue* jv){
  return jv->type == JSON_NUMBER;
}
bool json_isnull  (const JsonValue* jv){
  return jv->type == JSON_NULL;
}
bool json_isbool  (const JsonValue* jv){
  return jv->type == JSON_BOOL;
}
bool json_isarray (const JsonValue* jv){
  return jv->type == JSON_ARRAY;
}
bool json_isobject(const JsonValue* jv){
  return jv->type == JSON_OBJECT;
}

// as-functions
Option(string) json_asstring(const JsonValue* jv){
  if (!jv) return OPT_NONE(string);
  if (jv->type == JSON_STRING) return OPT_SOME(string, jv->as.str);
  return OPT_NONE(string);
}
Option(double) json_asnumber(const JsonValue* jv){
  if (!jv) return OPT_NONE(double);
  if (jv->type == JSON_NUMBER) return OPT_SOME(double, jv->as.number);
  return OPT_NONE(double);
}
Option(bool) json_asbool(const JsonValue* jv){
  if (!jv) return OPT_NONE(bool);
  if (jv->type == JSON_BOOL) return OPT_SOME(bool, jv->as.boolean);
  return OPT_NONE(bool);
}
Option(JsonArray) json_asarray(const JsonValue* jv){
  if (!jv) return OPT_NONE(JsonArray);
  if (jv->type == JSON_ARRAY) return OPT_SOME(JsonArray, jv->as.array);
  return OPT_NONE(JsonArray);
}
Option(JsonObject) json_asobject(const JsonValue* jv){
  if (!jv) return OPT_NONE(JsonObject);
  if (jv->type == JSON_OBJECT) return OPT_SOME(JsonObject, jv->as.object);
  return OPT_NONE(JsonObject);
}

#endif // JSON_IMPLEMENTATION

#endif // JSON_H
