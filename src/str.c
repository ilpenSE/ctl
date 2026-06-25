#include <str.h>
#include <array.h>
#include <basic.h>

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

#ifdef __StringView_defined
String str_from_sv_impl(const StringView* sv, StringMemory memory) {
  return str_newn_impl(sv->data, sv->len, memory);
}
#endif // __StringView_defined

String str_newn_impl(const char* buf, size_t len, StringMemory memory) {
  String s = {.memory=memory};
  size_t cap = len + 16; // this 16 is for pre-allocation
  if (!str_reserve(&s, cap)) return s;
  memcpy(s.data, buf, len);
  s.data[len] = '\0';
  s.len = len;
  return s;
}

String str_new_impl(const char* buf, StringMemory memory) {
  return str_newn_impl(buf, strlen(buf), memory);
}

void str_free(String* s) {
  if (!s->data) return;
  // Use free from stdlib.h if no custom free function preset.
  str_freer_t free_fn = s->memory.freer;
  if (free_fn == NULL) free_fn = free;
  free_fn(s->data);
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
  // Use realloc from stdlib.h if no custom realloc function preset.
  str_reallocator_t realloc_fn = s->memory.reallocator;
  if (realloc_fn == NULL) realloc_fn = realloc;

  size_t new_cap = s->len + 1;
  void* tmp = realloc_fn(s->data, new_cap);
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
  // Use realloc from stdlib.h if no custom realloc function preset.
  str_reallocator_t realloc_fn = s->memory.reallocator;
  if (realloc_fn == NULL) realloc_fn = realloc;

  char *tmp = (char*)realloc_fn(s->data, new_cap);
  if (!tmp) return false;

  s->data = tmp;
  s->cap = new_cap;
  return true;
}

bool str_join_cc(String* str, Array(cchar_ptr)* arr, char delim) {
  if (delim == '\0') return false;
  size_t len = arr_len(*arr);
  for (size_t i = 0; i < len; i++) {
    if (!str_cat(str, arr->items[i])) return false;
    if (i != len - 1)
      if (!str_append(str, delim)) return false;
  }
  return true;
}

bool str_join_c(String* str, Array(char_ptr)* arr, char delim) {
  Array(cchar_ptr) _arr = arr_cast(cchar_ptr, *arr);
  bool ok = str_join_cc(str, &_arr, delim);
  *arr = arr_cast(char_ptr, _arr);
  return ok;
}

bool str_catn(String* s, const char* buf, size_t bufsz) {
  if (!buf) return false;
  if (bufsz == 0) return true;
  if (!str_reserve(s, bufsz)) return false;

  memcpy(s->data + s->len, buf, bufsz);
  s->len += bufsz;
  s->data[s->len] = '\0';
  return true;
}

bool str_cat(String* s, const char* buf) {
  if (!buf) return false;
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

bool str_starts_withn(const String* str, const char* buf, size_t bufsz) {
  if (str->len == 0) return false;
  if (0 == bufsz || bufsz > str->len) return false;
  return memcmp(str->data, buf, bufsz) == 0;
}

bool str_starts_with(const String* str, const char* buf) {
  return str_starts_withn(str, buf, strlen(buf));
}

bool str_ends_withn(const String* str, const char* buf, size_t bufsz) {
  if (str->len == 0) return false;
  if (0 == bufsz || bufsz > str->len) return false;
  return memcmp(str->data + str->len - bufsz, buf, bufsz) == 0;
}

bool str_ends_with(const String* str, const char* buf) {
  return str_ends_withn(str, buf, strlen(buf));
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
