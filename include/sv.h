/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SV_H
#define SV_H

#ifdef __cplusplus
  #define SVDEF extern "C"
#else
  #define SVDEF extern
#endif

#include <string.h>
#include <stddef.h>
#include "basic.h"

#define SV_ARG(sv) (int)(sv).len, (sv).data
#define SV_FMT "%.*s"

#define sv_foreach(sv, it) \
  for (const char* it = (sv)->data; it < (sv)->data + (sv)->len; it++)

/*
  Usage: sv_from_str(s, .start = 0, .end = s->len)
  or: sv_from_str(s), sv_from_str(s, .end = 10) (default values'll be 0)
*/
#define svs(s, ...) \
  sv_from_str_impl((s), (StringSlice){.start=0, .end=(s)->len, ##__VA_ARGS__ })

/*
  Slices heap-allocated string, wrapper to sv_from_cstr
*/
SVDEF StringView sv_from_str_impl(const String* s, StringSlice slice);

#define svn(buf, len, ...) \
  sv_from_cstrn_impl((buf), (len), (StringSlice){.end=len-1,__VA_ARGS__})

/*
  Slices string in range [start, end) and makes new StringView
  Length, start and end are checked
*/
SVDEF StringView sv_from_cstrn_impl(const char* buf, size_t len, StringSlice slice);

#define sv(buf, ...) \
  sv_from_cstr_impl((buf), (StringSlice){.end=(strlen((buf))), ##__VA_ARGS__})

/*
  Produce StringView from zero-ended const strings without explicit start and end
*/
SVDEF StringView sv_from_cstr_impl(const char* buf, StringSlice slice);

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
SVDEF void sv_tostr(StringView sv, char* out);

/*
  Compares 2 string views and check if they're equals
  Basically: sv_cmp(lhs, rhs) == 0 like strcmp(lhs, rhs) == 0
*/
SVDEF bool sv_equals(StringView lhs, StringView rhs);
SVDEF bool sv_equals_cstr(StringView lhs, const char* rhs);

/*
  Compares 2 string views
  returns zero     if lhs == rhs
  returns positive if lhs >  rhs
  returns negative if lhs <  rhs
  First, diff lengths: lhs.len - rhs.len (will be pos if lhs's length is greater)
  Then check buffers byte by byte and if any different byte found in same length,
  return it. Simply: memcmp(lhs, rhs, lhs.len)
*/
SVDEF int sv_cmp(StringView lhs, StringView rhs);

/*
  Checks if the buffer which the string view looks at starts or ends with "buf"
  bufsz is strlen(buf)
  If bufsz is equal to 0 or greater than view's length, returns false
*/
SVDEF bool sv_starts_withn(StringView view, const char* buf, size_t bufsz);
SVDEF bool sv_starts_with(StringView view, const char* buf);
SVDEF bool sv_ends_withn(StringView view, const char* buf, size_t bufsz);
SVDEF bool sv_ends_with(StringView view, const char* buf);

/*
  Clears the string view, making length zero
  and pointer to null
*/
SVDEF void sv_clear(StringView *sv);

/*
  Check if data pointer points to zero and length is also zero
*/
SVDEF bool sv_isempty(StringView sv);

#endif /* SV_H */
