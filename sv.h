#ifndef SV_H
#define SV_H

#ifdef __cplusplus
  #define SVDEF extern "C"
#else
  #define SVDEF extern
#endif

#include <stddef.h>

#ifndef __StringView_defined
#define __StringView_defined
typedef struct {
  const char* data; /* not null terminated */
  size_t len;
} StringView;
#endif

#ifndef __String_defined
#define __String_defined
typedef struct {
  char* data;
  size_t len; /* does not include \0 */
  size_t cap;
} String;
#endif

#ifndef __StringSlice_defined
#define __StringSlice_defined
typedef struct {
  size_t start;
  size_t end;
} StringSlice;
#endif

#ifndef __uchar_t_defined
typedef unsigned char uchar_t;
#define __uchar_t_defined
#endif

#define SV_ARG(sv) (int)(sv)->len, (sv)->data
#define SV_FMT "%.*s"

/*
  Usage: sv_from_str(s, .start = 0, .end = s->len)
  or: sv_from_str(s), sv_from_str(s, .end = 10) (default values'll be 0)
*/
#define sv_from_str(s, ...) sv_from_str_impl((s), (StringSlice){ __VA_ARGS__ })

/*
  Slices heap-allocated string, wrapper to sv_from_cstr
*/
SVDEF StringView sv_from_str_impl(const String* s, StringSlice slice);

#define sv_from_cstre(buf, len, ...) \
  sv_from_cstre_impl((buf), (len), (StringSlice){ __VA_ARGS__ })

#define sv_foreach(sv, it) \
  for (const char* it = (sv)->data; it < (sv)->data + (sv)->len; it++)

/*
  Slices string in range [start, end] and makes new StringView
  Length, start and end are checked
*/
SVDEF StringView sv_from_cstre_impl(const char* buf, size_t len, StringSlice slice);

/*
  Produce StringView from zero-ended const strings without explicit start and end
*/
SVDEF StringView sv_from_cstr(const char* buf);

/*
  Take sub string from string view and return the sub string
*/
SVDEF StringView sv_substr(StringView sv, size_t start, size_t len);

/*
  Trims the string view from left or right or both sides
*/
SVDEF StringView sv_trim_left(StringView sv);
SVDEF StringView sv_trim_right(StringView sv);
SVDEF StringView sv_trim(StringView sv);

/*
  Chop n characters from left or right
  modifies given sv and constructs removed part then returns it
*/
SVDEF StringView sv_chop_left(StringView* sv, size_t n);
SVDEF StringView sv_chop_right(StringView* sv, size_t n);

/* Synonyms for chops */
#define sv_remove_prefix sv_chop_left
#define sv_remove_suffix sv_chop_right

/*
  Start from left, chop until hitting the first delimiter character
  Removes chopped part and delimiter char from string view
  Returns that chopped part
  If no delimiter char found, empty StringView'll be returned
*/
SVDEF StringView sv_chop_by_delim(StringView* sv, char delim);

/*
  Chop string view, starting from left, by a function
  Function can be from ctype.h (isspace, isblank etc.)
*/
SVDEF StringView sv_chop_by_func(StringView* sv, int (*func)(int));

/*
  Writes sv's data with length into zero-terminated string
  DICLAIMER: make sure that out buffer's size is enough
  otherwise you will get buffer overflow
*/
SVDEF void sv_tostr(const StringView* sv, char* out);

/*
  Compares 2 string views and check if they're equals
  Basically: sv_cmp(lhs, rhs) == 0 like strcmp(lhs, rhs) == 0
*/
SVDEF bool sv_equals(const StringView* lhs, const StringView* rhs);

/*
  Compares 2 string views
  returns zero     if lhs == rhs
  returns positive if lhs >  rhs
  returns negative if lhs <  rhs
  First, diff lengths: lhs.len - rhs.len (will be pos if lhs's length is greater)
  Then check buffers byte by byte and if any different byte found in same length,
  return it. Simply: memcmp(lhs, rhs, lhs.len)
*/
SVDEF int sv_cmp(const StringView* lhs, const StringView* rhs);

/*
  Clears the string view, making length zero
  and pointer to null
*/
SVDEF void sv_clear(StringView *sv);

/*
  Check if data pointer points to zero and length is also zero
*/
SVDEF bool sv_isempty(const StringView* sv);

// IMPLEMENTATION BEGIN
#ifdef SV_IMPLEMENTATION

#include <string.h>
#include <ctype.h>

StringView sv_from_cstre_impl(const char* buf, size_t len, StringSlice slice) {
  StringView sv = {0};
  if (slice.start > slice.end || slice.start > len || slice.end > len) return sv;
  sv.data = buf + slice.start;
  sv.len = slice.end - slice.start;
  return sv;
}

StringView sv_from_cstr(const char* buf) {
  size_t buflen = strlen(buf);
  return sv_from_cstre(buf, buflen, .start = 0, .end = buflen);
}

StringView sv_from_str_impl(const String* s, StringSlice slice) {
  return sv_from_cstre_impl(s->data, s->len, slice);
}

void sv_tostr(const StringView* sv, char* out) {
  if (!out || !sv) return;
  memcpy(out, sv->data, sv->len);
  out[sv->len] = '\0';
}

void sv_clear(StringView *sv) {
  if (sv == NULL) return;
  sv->len = 0;
  sv->data = "";
}

bool sv_isempty(const StringView* sv) {
  return sv->len == 0 && (!sv->data ? true : *sv->data == 0);
}

int sv_cmp(const StringView* lhs, const StringView* rhs) {
  if (lhs == rhs) return 0;
  int sdiff = (int)lhs->len - (int)rhs->len;
  if (sdiff != 0) return sdiff; // size diff
  return memcmp(lhs->data, rhs->data, lhs->len);
}

bool sv_equals(const StringView* lhs, const StringView* rhs) {
  return sv_cmp(lhs, rhs) == 0;
}

StringView sv_chop_by_delim(StringView* sv, char delim) {
  StringView chopped = {0};
  for (size_t i = 0; i < sv->len; i++) {
    if (sv->data[i] == delim) {
      // Construct chopped part
      chopped.len = i;
      chopped.data = sv->data;
      // TRIM orig
      sv->len -= i + 1;
      if (i == sv->len - 1) {
        sv->data = NULL;
      } else {
        sv->data += i + 1;
      }
      break;
    }
  }
  return chopped;
}

StringView sv_chop_by_func(StringView* sv, int (*func)(int)) {
  StringView chopped = {0};
  for (size_t i = 0; i < sv->len; i++) {
    if (func((uchar_t)sv->data[i])) {
      // Construct chopped part
      chopped.len = i;
      chopped.data = sv->data;
      // TRIM orig
      sv->len -= i + 1;
      if (i == sv->len - 1) {
        sv->data = NULL;
      } else {
        sv->data += i + 1;
      }
      break;
    }
  }
  return chopped;
}

StringView sv_chop_left(StringView* sv, size_t n) {
  if (n >= sv->len) return (StringView) {0};
  StringView res = {.data=sv->data, .len=n};
  sv->data += n;
  sv->len -= n;
  return res;
}

StringView sv_chop_right(StringView* sv, size_t n) {
  StringView res = {.data=sv->data + sv->len - n, .len=n};
  sv->len -= n;
  return res;
}

StringView sv_trim_left(StringView sv) {
  while (sv.len > 0 && isspace((uchar_t)sv.data[0])) {
    sv.data++;
    sv.len--;
  }
  return sv;
}

StringView sv_trim_right(StringView sv) {
  while (sv.len > 0 && isspace((uchar_t)sv.data[sv.len - 1])){
    sv.len--;
  }
  return sv;
}

StringView sv_trim(StringView sv) {
  return sv_trim_right(sv_trim_left(sv));
}

#endif // SV_IMPLEMENTATION
// IMPLEMENTATION END
#endif /* SV_H */

/*
  The MIT License
  Copyright (c) 2026 ilpeN

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
