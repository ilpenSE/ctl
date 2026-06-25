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
DECL_ARRAY(JsonPair);
#endif /* __Array_JsonPair_defined */

#ifndef __Array_JsonValue_defined
#define __Array_JsonValue_defined
DECL_ARRAY(JsonValue);
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
DECL_RESULT(Json);
#endif /* __Result_Json_defined */

#ifndef __Result_JsonValue_defined
#define __Result_JsonValue_defined
DECL_RESULT(JsonValue);
#endif /* __Result_JsonValue_defined */

#ifndef __Result_JsonObject_defined
#define __Result_JsonObject_defined
DECL_RESULT(JsonObject);
#endif /* __Result_JsonObject_defined */

#ifndef __Result_JsonArray_defined
#define __Result_JsonArray_defined
DECL_RESULT(JsonArray);
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

#endif /* JSON_H */
