#ifndef STR_H
#define STR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
#define STRDEF extern "C"
#else
#define STRDEF extern
#endif

#ifndef __String_defined
#define __String_defined
typedef struct {
  char* data;
  size_t len; // does not include \0
  size_t cap;
} String;
#endif // __String_defined

#ifndef __StringView_defined
#define __StringView_defined
typedef struct {
  const char* data; // not null terminated
  size_t len;
} StringView;
#endif // __StringView_defined

#ifndef __uchar_t_defined
typedef unsigned char uchar_t;
#define __uchar_t_defined
#endif

#define str_foreach(s, name) for(char* name = (s)->data; name < (s)->data + (s)->len; name++)

/*
  Functions that take String context
  as first argument and pointer DO NOT CHECK NULL CONTEXT!
  Like: bool str_reserve(NULL, 10); RESULTS WITH UNDEFINED BEHAVIOR!
  It's your responsibility to check them
*/

// Bridge functions between str.h and sv.h
// String must be defined, including str.h defines it
#ifdef __StringView_defined
/*
  Converts StringView into heap-allocated String
  Allocates memory for String, copies StringView's data and length
*/
STRDEF String str_from_sv(const StringView* sv);
#endif // __StringView_defined

/*
  Makes and returns string struct from C-strings with length
  If you have len value, use this
  But if you dont have and if you're using strlen, you can use
  str_from_cstr, it does strlen
 */
STRDEF String str_newn(const char* buf, size_t len);

/*
  str_newn with no len value, it calculates len via strlen
  If you dont have your String's length value or just dont want
  to use strlen by yourself, you can use this
 */
STRDEF String str_new(const char* buf);

/*
  Reservers needed memory for the String
  It usually multiplies it by 2 if String is too long
  Extra is additional bytes to append
 */
STRDEF bool str_reserve(String* s, size_t extra);

/*
  Shrinks the memory (cap field) to length + 1
*/
STRDEF bool str_shrink_to_fit(String* s);

/*
  Returns char in that String by a pos value
  pos stands for position (the index, starts from zero)
 */
STRDEF char str_idx(const String* s, size_t pos);

/*
  It appends char into that String
  Automatically puts \0 at the end
 */
STRDEF bool str_append(String* s, char c);

/*
  Str_cat but with C-type Strings (char*)
  "bufsz" is length of buf.
*/
STRDEF bool str_catn(String* s, const char* buf, size_t bufsz);

/*
  Wrapper of str_append_manyn, calls strlen on buf
 */
STRDEF bool str_cat(String* s, const char* buf);

/*
  It concatenates dest String and src String
  It puts src String's chars into dest String and
  puts \0 at the end and updates count and capacity (if needed)
*/
STRDEF bool str_cat_str(String* dest, const String* src);

/*
  It gets "data" field in that String
  You can always use s->data or s.data
  But this checks NULL or empty conditions
*/
STRDEF char* str_to_cstr(const String* s);

/*
  Compares 2 strings their lengths and buffers
  zero     if they're equal (length and buffer)
  positive if lhs > rhs (length or buffer)
  negative if rhs > lhs
*/
STRDEF int str_cmp(const String* lhs, const String* rhs);

/*
  Calls str_cmp with check if return value is 0 or not
*/
STRDEF bool str_equals(const String* lhs, const String* rhs);

/*
  Free the String
*/
STRDEF void str_free(String* s);

/*
  It clears the String, sets len to zero and make data \0
  Capacity still there and this function DOES NOT FREE THE MEMORY!
*/
STRDEF void str_clear(String* s);

/*
  Trims the whitespaces given string from left or right or both sides
*/
STRDEF void str_trim(String* s);
STRDEF void str_trim_left(String* s);
STRDEF void str_trim_right(String* s);

/*
  Closes the String if not closed or corrupted
  Inserts \0 at the end (depends on count field)
*/
STRDEF void str_close(String* s);

/*
  Checks if String is ended with \0
*/
STRDEF bool str_is_closed(const String* s);

/*
  Repeats String with provided count, modifies original String
  It appends that String at the end count - 1 times
  that means if you provide "xx" as String and 4 as count,
  you get: "xxxxxxxx" (=4*"xx")
*/
STRDEF void str_repeat(String* s, size_t count);

/*
  Formats the String with given format String and variadics
  Acts like running a temporary snprintf and applying it to String
*/
STRDEF void str_format_into(String* s, const char* fmt, ...);

/*
  Makes all alpha characters in that String lowercased - ASCII only
 */
STRDEF void str_tolower(String* s);

/*
  Makes all alpha characters in that String uppercased - ASCII only
*/
STRDEF void str_toupper(String* s);

/*
  Checks if all of chars in that String is alpha - ASCII only
 */
STRDEF bool str_isalpha(const String* s);

/*
  Checks if all chars in String is alphanumeric - ASCII only
 */
STRDEF bool str_isalphanum(const String* s);

/*
  Capitalizes String like this:
  hello -> Hello
  heLLo -> Hello
  hell1o -> Hello1o
  Doesn't touch non-alpha chars
  Uppers first char if it is alpha and lowers the rest (alpha ones)
  1hello -> 1hello (because 1 is not alpha)
 */
STRDEF void str_capitalize(String* s);

// IMPLEMENTATION BEGIN
#ifdef STR_IMPLEMENTATION

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

#ifdef __StringView_defined
String str_from_sv(const StringView* sv) {
  return str_newn(sv->data, sv->len);
}
#endif // __StringView_defined

String str_newn(const char* buf, size_t len) {
  String s = {0};
  size_t cap = len + 16; // this 16 is for pre-allocation
  if (!str_reserve(&s, cap)) return s;
  memcpy(s.data, buf, len);
  s.data[len] = '\0';
  s.len = len;
  return s;
}

String str_new(const char* buf) {
  return str_newn(buf, strlen(buf));
}

void str_free(String* s) {
  if (!s->data) return;
  free(s->data);
  s->data = NULL;
  s->len = 0;
  s->cap = 0;
}

void str_clear(String *s) {
  s->len = 0;
  if (s->data) {
    s->data[0] = '\0';
  }
}

bool str_shrink_to_fit(String* s) {
  size_t new_cap = s->len + 1;
  void* tmp = realloc(s->data, new_cap);
  if (!tmp) return false;
  else s->data = (char*)tmp;
  s->cap = new_cap;
  return true;
}

char str_idx(const String* s, size_t pos) {
  if (!s->data) return '\0';
  if (pos >= s->len) return '\0';
  return s->data[pos];
}

bool str_append(String* s, char c) {
  if (!str_reserve(s, 1)) return false;

  s->data[s->len++] = c;
  s->data[s->len] = '\0';
  return true;
}

bool str_reserve(String* s, size_t extra) {
  size_t sum = extra + s->len + 1;

  // if sum is enough to fit cap, dont update cap
  if (sum <= s->cap) return true;

  // for size_t overflows
  if (sum > SIZE_MAX / 2) return false;

  // sum does NOT fit
  size_t new_cap = sum * 2;
  char *tmp = (char*)realloc(s->data, new_cap);
  if (!tmp) return false;

  s->data = tmp;
  s->cap = new_cap;
  return true;
}

bool str_catn(String* s, const char* buf, size_t bufsz) {
  if (!buf || !s->data) return false;
  if (bufsz == 0) return true;

  if (!str_reserve(s, bufsz)) return false;

  memcpy(s->data + s->len, buf, bufsz);
  s->len += bufsz;
  s->data[s->len] = '\0';
  return true;
}

bool str_cat(String* s, const char* buf) {
  return str_catn(s, buf, strlen(buf));
}

bool str_cat_str(String* s, const String* c) {
  return str_catn(s, c->data, c->len);
}

int str_cmp(const String* lhs, const String* rhs) {
  if (lhs == rhs) return 0;
  int sdiff = (int)lhs->len - (int)rhs->len;
  if (sdiff != 0) return sdiff; // size diff
  return memcmp(lhs->data, rhs->data, lhs->len);
}

bool str_equals(const String* lhs, const String* rhs) {
  return str_cmp(lhs, rhs) == 0;
}

void str_trim_left(String* s) {
  // count how many spaces at the begin
  size_t start = 0;
  while (start < s->len && isspace((uchar_t)s->data[start])) {
    start += 1;
  }

  // move String by "start" characters
  if (start != 0) {
    memmove(s->data, s->data + start, s->len - start + 1);
    s->len -= start;
  }
}

void str_trim_right(String* s) {
  while (s->len != 0 && isspace((uchar_t)s->data[s->len - 1])) {
    s->len -= 1;
  }
  s->data[s->len] = '\0';
}

void str_trim(String* s) {
  str_trim_left(s);
  str_trim_right(s);
}

void str_repeat(String* s, size_t multiplier) {
  if (multiplier == 1) return;
  if (multiplier == 0) return;

  size_t new_cnt = s->len * multiplier;
  if (!str_reserve(s, new_cnt - s->len)) return;

  for (size_t i = 1; i < multiplier; ++i) {
    memmove(s->data + (i * s->len), s->data, s->len);
  }
  
  s->len = new_cnt;
  s->data[new_cnt] = '\0';
}

void str_tolower(String* s) {
  if (!s->data) return;
  for (size_t i = 0; i < s->len; ++i) {
    uchar_t c = (uchar_t)s->data[i];
    uchar_t is_upper = (c >= 'A') & (c <= 'Z');
    s->data[i] += is_upper * 32;
  }
}

void str_toupper(String* s) {
  if (!s->data) return;
  for (size_t i = 0; i < s->len; ++i) {
    uchar_t c = (uchar_t)s->data[i];
    uchar_t is_lower = (c >= 'a') & (c <= 'z');
    s->data[i] -= is_lower * 32;
  }
}

void str_capitalize(String* s) {
  if (!s->data) return;
  for (size_t i = 0; i < s->len; ++i) {
    uchar_t c = (uchar_t)s->data[i];
    if (i == 0) {
      uchar_t is_lower = (c >= 'a') & (c <= 'z');
      s->data[i] -= is_lower * 32;
    } else {
      uchar_t is_upper = (c >= 'A') & (c <= 'Z');
      s->data[i] += is_upper * 32;
    }
  }
}

void str_format_into(String* s, const char* fmt, ...) {
  if (!s->data || s->cap == 0) return;

  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(s->data, s->cap, fmt, ap);
  va_end(ap);

  if (n < 0) {
    s->len = 0;
  } else if ((size_t)n >= s->cap) {
    size_t cap = s->cap;
    if (cap >= 1) {
      s->data[cap - 1] = '\0'; // guarenttes \0
      s->len = cap - 1;
    } else {
      s->len = 0;
    }
  } else {
    s->len = (size_t)n;
  }
}

bool str_isalpha(const String* s) {
  if (!s->data) return false;
  for (size_t i = 0; i < s->len; ++i) {
    uchar_t c = (uchar_t) s->data[i];
    uchar_t is_lower = (c >= 'a') & (c <= 'z');
    uchar_t is_upper = (c >= 'A') & (c <= 'Z');
    // is not alpha mask
    uchar_t mask = !(is_lower | is_upper);
    if (mask) return false;
  }
  return true;
}

bool str_isalphanum(const String* s) {
  if (!s->data) return false;
  for (size_t i = 0; i < s->len; ++i) {
    uchar_t c = (uchar_t) s->data[i];
    uchar_t is_lower = (c >= 'a') & (c <= 'z');
    uchar_t is_upper = (c >= 'A') & (c <= 'Z');
    uchar_t is_num = (c >= '0') & (c <= '9');
    // is not alphanumeric mask
    uchar_t mask = !(is_lower | is_upper | is_num);
    if (mask) return false;
  }
  return true;
}

void str_close(String* s) {
  if (!s->data) return;
  s->data[s->len] = '\0';
}

bool str_is_closed(const String* s) {
  return s->data[s->len] == '\0' ? true : false;
}

char* str_to_cstr(const String* s) {
  return s->data;
}

#endif // STR_IMPLEMENTATION
// IMPLEMENTATION END
#endif // STR_H
